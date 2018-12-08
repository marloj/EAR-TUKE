# Search Network

![recognition network](../images/network.png)

The search network is represented using WFSTs which is a state machine consisting of states and transitions between them. Each transition has input and output symbols with weights. Each search network contains one initial state (marked bold) and one or more final states (marked with double line). The WFSTs are used because of their ability to translate input sequence of symbols to output sequence of other symbols, in our case from input acoustic observations to names of the detected acoustic events.

This representation of the search network has its advantages. The first advantage is that the decoder does not need to know any information provided in the search network as long as the format of the input symbols in the network is coordinated with the acoustic models. This also means that the set of desired events to be detected can be easily adjusted and we can include additional statistical information that needs to be taken into account by the decoding process.


The decoder is based on Hidden Markov Models (HMMs). An HMM  consists of two parts. The topology of the model is represented by the transition model and probability density functions (PDFs) represent observations probabilities.
The WFSTs are used for the representation of the topology while the transitions contain probabilities of acoustic model transitions and input symbols are informing about PDFs that need to be used for scoring the input vectors. The output symbols on the transitions contain names of the acoustic events. The advantage of representing the topology of HMMs by WFSTs is that the search network can support different topology and number of states for different acoustic events without prior knowledge of the decoder.

The search network for our system has to be generated before the process of detection, meaning that the network representation should be static. As the search network for this kind of task tends to be relatively small (a few dozens of states), the static representation does not impose any limitation on the usage of the system even on embedded devices with limited resources. The small size of the network also supports fast rearrangement of the network on demand. Another advantage of the static search network is that decoder does not need to deal with the composition of it on-the-fly and so the decoder can work faster.

In order to do the detection and classification of an acoustic event, the search network contains models not only for acoustic events, but also for background sounds. The network contains three events: gunshot, glass breaking and one background model,  displayed as the output symbols on the transitions. Symbol "*<eps>*" describes empty output symbol. The input symbols are used as information for scoring process and basically represent the states (Gaussian Mixtures) in HMM acoustic models. The weights on transitions represent transition probabilities between states  of the HMM acoustic model. This search network is stored in a binary file that includes the acoustic model information about PDFs allowing  faster loading of all required information into the memory.

# Search (detection) process

![search process](../images/net_fnc.png)

The task of the decoder is to find the best  sequence of the  acoustic events based on the input feature vectors extracted from input audio signals. The decoder inspects the search network using feature vectors as guidance in synchronous manner where one step in the search network is performed for one feature vector.

During the search the intermediate scores composed from transition probabilities and acoustic score from PDFs are computed. The result is defined as the most probable path in the search network. The Viterbi decoding criterion is used and implemented in the form of token passing algorithm in a similar way  it is done in Hidden Markov Model Toolkit (HTK). The token is an object that can travel through the search network and remembers all required intermediate results for constructing the final sequence.  In the case of our system, the token remembers these  four elements during its travel through the search network:
1. Accumulated transition score
2. Accumulated acoustic score
3. Last output symbol found
4. Pointer to a token with last non-empty output symbol

When the token travels through the search network, the weight on the transitions is added to the accumulated transition score. In a similar way, accumulated acoustic score is calculated as Mahalanobis distance of the input feature vector to the PDFs with incoming feature vector according to the input symbol $n$ on the same transition.

These accumulated scores are used together for comparing tokens to each other according to Viterbi decoding criterion (the most probable token wins). The result is then defined as sequence of states with maximum probability.

The next information that needs to be found together with the accumulated score are the output symbols on the transitions. The best output symbol sequence with the highest score gives us the result of the decoding process. The last two items of the token are used exactly for that purpose. Each token  remembers the last output symbol that it passed during its travel through the search network along with the pointer to a token with previously passed  output symbol. The pointer allows the tokens to chain themselves in order to find the hypotheses of the decoding process and finally the result of the whole decoding.

The black bullets represent the tokens in the search network, while the black arrows represent the pointer to token with last non-empty output symbol. The probabilities are depicted in logarithmic scale without obvious minus sign. In the  beginning only single token exists in the search network and it is placed into the initial state.

Although logical assumption is to place the tokens where  the information for decoding is, as it is in case of other systems like HTK, in our case the tokens are placed in states regardless of the information on transitions.

The tokens are traveling from state to state. With an incoming input feature vector the tokens are copying themselves to all consecutive states accumulating the total score on the transitions and remembering the output symbols. Each new copy of a token, before passing through a transition, inherits from previous one its score and pointer to a token with last non-empty output symbol if there has been any (in essence remembering the way back).
If the new token passes a non-empty output symbol on its transition to the next state, it remembers it and  the subsequent tokens copied from this one will inherit a pointer to this token.

By using the pointer to tokens with the last found non-empty output symbol, the hypotheses are maintained during the decoding process. When the last token in the chain is removed from the decoding process according Viterbi decoding criterion, the whole hypothesis (the whole chain) should be also removed.
Any token in the hypothesis chain can be part of another chain, so each token contains a reference counter.

After a token is removed  from the search network (does not reside in any state), it is still kept in the memory in case there were any other tokens referencing to it.
If the reference counter drops to zero, the token is completely  removed from the decoding process. To retrieve partially or fully decoded result, the token from the final state can be taken.

The chain where the final state belongs represents the result of decoding process. This solution does not affect the speed of the decoder and it provides very good memory and stability increase for short acoustic events detection  without significant accuracy change as it is described in the next subsection.

# Decoding control mechanism

![decoding control](../images/dp.png)

This last part of the EAR-TUKE system is a small but important block. Thanks to this block, the continuous detection and classification is achieved. This block actually does more than that, it also saves computational resources.

As the preprocessing block does not provide any information to distinguish acoustic events from background sounds, the detection and the following classification needs to be done in a different way to where two consecutive steps of event detection and event classification are used.  In the usual approach the decoding results are collected at the end of the recording. Unfortunately, this cannot be done for a continuous  audio stream input because the audio stream does not end, and there is no block which cuts some part of the stream (as it is done when an event detection block exists).

If we use our modified decoder and the presented search network for processing and recording, the output will be a sequence of acoustic events detected.
We have stated at the beginning of this paper that we want to design the system to continuously monitor the audio stream. To achieve this, a block for controlling the decoder based on the hypothesis evaluation was designed. The function of this block is to look at the end of the current decoding hypothesis (the one represented by a token in the final state) after each input feature vector. When the hypothesis meets preset conditions, the whole decoding process can be quickly reset, meaning that all hypotheses will be removed and all resources released. The new decoding process will start immediately afterwards.

The conditions were chosen so that they take advantage of the background models in the search network. We found  that it is safe to reset the decoder when the most probable hypothesis reaches the background model. To be sure that this hypothesis is not a temporary glitch, there is also a condition concerning the duration of the hypothesis. Empirically, the optimal time was finally set to 100 milliseconds. A safe reset of the decoder means that it does not produce an error or a false hypothesis afterwards. The results of  experiments with  continuous resetting of the decoder and without it were the same.
