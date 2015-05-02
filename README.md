It is a kind of Wordament Solver. You have to just put Wordament (an Android Based Application http://goo.gl/Tt0Y8o) screen in front of webcam and all the possible words that can be made out of it will be listed.
It uses OpenCV for segmenting Image and OCR is done by Tesseract Library.
Firstly I have taken list of around 1 million popular word sorted in alphabetical order and thern I saved them in Dictionary using Tries Data Structure. And after segmenting Image and after OCR whatever result I am getting , I have applied DFS on it and matched all the possible word that are their in my dictionary.

Steps:
1. Install OpenCv 
link http://docs.opencv.org/doc/tutorials/introduction/linux_install/linux_install.html

2.Install Tesseract(OCR)  
link  http://ubuntu.flowconsult.at/linux/ocr-tesseract-text-recognition-ubuntu-14-04/

3.type make command 

4. Put your Phone screen in front of webcam and press any key.
It will show all the possible words which can be made.
