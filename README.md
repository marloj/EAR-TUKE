
EAR-TUKE
========

System for detection and acoustic events like gushots and glass breaking.

Licence
--------
GPLv3

Building EAR-TUKE
-----------------

make

Running the example
--------------------

- offline example
The detection results are displayed on the end of the recording processing.

		./Ear ./Example/example.cfg ./Example/example.wav

- online example
For online example of the detection, please change following line in the `./Example/example.cfg` file to enable online mode and run the above command line again. The results should be displayed as they are detected in the input file

		ONLINE T

- online live example
For online live example (reading data from microphone) run the system without any input file. Please note that in order to see the events the online mode needs to be set in `./Example/example.cfg`.

		./Ear ./Example/example.cfg

Acoustic model preparation
--------------------------

Currently HTK models are supported. Please follow the steps in order to create the recognition network:

1. Create dictionary file in the form
		output_label_name	probability_of_the_event	model_name_of_the_event
	
Different models can be grouped in one category (see `./Example/melspec_1state_256pdf/dict.txt`). For example the background models trained in different conditions into one background output name. If there is no special probability of any event, zero value can be used to make each of the events equal probable.

2. Run the following command
		./Compile ./Example/melspec_1state_256pdf/model.mmf ./Example/melspec_1state_256pdf/dict.txt ./Example/melspec_1state_256pdf/model`

It will create a `model.fst`, `model.isym`, `model.osym`, `model.bin` , and `model.idx` for you. The first three files are FST, input and output symbols forming only the recognition network (see file `./Example/melspec_1state_256pdf/model.pdf` for the graphical representation of the network). The last two files are used by the system.

Citation
---------

If you use data or code, please cite the following:

Lojka, M., Pleva, M., Kiktová, E., Juhár J., Čižmár A.", Efficient acoustic detector of gunshots and glass breaking, Multimedia Tools and Applications, 2016, https://doi.org/10.1007/s11042-015-2903-z

.. code-block:: none

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
