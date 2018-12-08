# Signal preprocessing chain

![picture of front-end](../images/frontend.png)

The processing of the input signal (extraction of the feature vectors) according configuration. The EAR-TUKE frontend is capable of extraction of the MELSPEC, FBANK and MFCC coefficients.
1. **CFrame** Segmenting the input signal to the overlapping frames.
2. **CEnergy** Extraction of the raw energy before application of the preemphasis (if required)
3. **CPreem** Preemphasis of the signal to emphasize higher frequencies
4. **CWindow** Apply window function (this is usually needed before spectral analysis)
5. **CForier** Do spectral analysis of the input frame
6. **CMelBank** Apply filters on the spectrum, get magnitude of each filter (MELSPEC coefficients, log(MELSPEC) = FBANK coefficients)
7. **CZeroCoef** MFCC zero coefficient
8. **CDct** Cosine transform (Cepstral coefficents)
9. **CLifter** Lifting (filtering in cepstral) the coeffients
10. **CConcat** Concatenating the zero coefficient if required by configuration
11. **CConcat** Concatenating the energy or raw energy to the resulting coefficients
12. **CDelta** Computing delta coefficients of first order between frames
13. **CDelta** Computing acceleration coefficients (delta coefficients of second order) between frames
14. **CDMN** Computing and removing cepstral mean from the coefficients.
