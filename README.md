
EAR-TUKE
========

Acoustic event detection system of gunshots and glass breaking.

Licence
--------
GPLv3

Building EAR-TUKE
-----------------

make

Running the example
--------------------

- Off-line example: 
The detection results will be displayed at the end of the recording processing.

		./Ear ./Example/example.cfg ./Example/example.wav

- On-line example: 
The on-line detection can be enabled by changing following line in the `./Example/example.cfg`.

		ONLINE T

Run the previous command-line again. The results will be displayed right as the event appears in the input recording.

- On-line live example:
To run the example using a microphone input, run the command-line without any input file. To see the results of detection, the online mode needs to be enabled as in the previous case.

		./Ear ./Example/example.cfg

Acoustic model preparation
--------------------------

Currently only HTK models are supported. The model needs to be converted to binary form that also contains recognition network using following steps:

1. Create dictionary file in the form
		
		output_label_name	probability_of_the_event	model_name_of_the_event
	
The `output_label_name` is used as output in case of the event detection that is modeled by `model_name_of_the_event`. There can be more than one acoustic model under the same output label (see `./Example/melspec_1state_256pdf/dict.txt`). For example, there can be more background models trained on different conditions, but grouped in one category and outputted under the same output label.

2. Run the following command

		./Compile ./Example/melspec_1state_256pdf/model.mmf ./Example/melspec_1state_256pdf/dict.txt ./Example/melspec_1state_256pdf/model

It will create files `model.fst`, `model.isym`, `model.osym`, `model.bin` , and `model.idx`. The first three files are not used by the system, they are just debugging output of the recognition network (the transducer, input and output symbols). For graphical representation see `./Example/melspec_1state_256pdf/model.pdf`. The important files are the last two of them. `model.bin` contains network definition and the acoustic model as well. As the system is working with id numbers istead of the event names, the `model.idx` contains the mapping between the two.

3. Change the configuration file

Update the configuration file to read the converted acoustic model with recognition network and the mapping file (the `MODEL_BIN_FILE` and `MODEL_IDX_FILE`). When the online detection mode is desirable, the parameter `BCG_IDX` needs to be changed to match the number of the background model in the mapping file.

Citation
---------

If you use data or code, please cite the following:

Lojka, M., Pleva, M., Kiktová, E., Juhár J., Čižmár A.", Efficient acoustic detector of gunshots and glass breaking, Multimedia Tools and Applications, 2016, https://doi.org/10.1007/s11042-015-2903-z

	@Article{Lojka2016,
	 author="Lojka, Martin and Pleva, Mat{\'u}{\v{s}} and Kiktov{\'a}, Eva and Juh{\'a}r, Jozef and {\v{C}}i{\v{z}}m{\'a}r, Anton",
	 title="Efficient acoustic detector of gunshots and glass breaking",
	 journal="Multimedia Tools and Applications",
	 year="2016",
	 month="Sep",
 	 day="01",
	 volume="75",
	 number="17",
	 pages="10441--10469",
	 abstract="An efficient acoustic events detection system EAR-TUKE is presented in this paper. The system is capable of processing continuous input audio stream in order to detect potentially dangerous acoustic events, specifically gunshots or breaking glass. The system is programmed entirely in C++ language (core math. functions in C) and was designed to be self sufficient without requiring additional dependencies. In the design and development process the main focus was put on easy support of new acoustic events detection, low memory profile, low computational requirements to operate on devices with low resources, and on long-term operation and continuous input stream monitoring without any maintenance. In order to satisfy these requirements on the system, EAR-TUKE is based on a custom approach to detection and classification of acoustic events. The system is using acoustic models of events based on Hidden Markov Models (HMMs) and a modified Viterbi decoding process with an additional module to allow continuous monitoring. These features in combination with Weighted Finite-State Transducers (WFSTs) for the search network representation fulfill the easy extensibility requirement. Extraction algorithms for Mel-Frequency Cepstral Coefficients (MFCC), Frequency Bank Coefficients (FBANK) and Mel-Spectral Coefficients (MELSPEC) are also included in the preprocessing part. The system contains Cepstral Mean Normalization (CMN) and our proposed removal of basic coefficients from feature vectors to increase robustness. This paper also presents the development process and results evaluating the final design of the system.",
	 issn="1573-7721",
	 doi="10.1007/s11042-015-2903-z",
	 url="https://doi.org/10.1007/s11042-015-2903-z"
	 }
