#include <iostream>
#include <chrono>
#include <thread>

#include <AudioUtilities/InputStream.hpp>
#include <AudioUtilities/OutputStream.hpp>
#include <AudioUtilities/ChooseDevices.hpp>

#include <Vocoder/TransportVocoder.hpp>

int main() {
  int hopSize = 1024;
  int windowSize = 4096;
  int sampleRate = 44100;
  int numOutputChannels = 2;
  int numInputChannels = 4;

  int inputDeviceNum;
  int outputDeviceNum;

  chooseDevices(
      numInputChannels,
      numOutputChannels,
      sampleRate,
      inputDeviceNum,
      outputDeviceNum
      );

  OutputStream outputStream(
      numOutputChannels, 
      hopSize,
      sampleRate,
      outputDeviceNum
      );

  double * output[2];

  TransportVocoder leftVocoder(hopSize, windowSize);
  TransportVocoder rightVocoder(hopSize, windowSize);

  double interpolationFactor = 0.5;

  // make handler function
  std::function<HandleInputBufferFunction> handler =
    [
    &outputStream, 
    &output,
    &leftVocoder,
    &rightVocoder,
    &interpolationFactor
      ](double ** audio) {

    // Vocode
    leftVocoder.processFrame(
        audio[0], 
        audio[2], 
        interpolationFactor,
        output[0]
        );
    rightVocoder.processFrame(
        audio[1], 
        audio[3], 
        interpolationFactor,
        output[1]
        );

    outputStream.write(output);
  };

  // open stream
  InputStream inputStream(
      &handler, 
      numInputChannels, 
      hopSize,
      sampleRate,
      inputDeviceNum
      );

  // Wait for user to quit
  std::string userInput;
  std::cout << std::endl;
  while (userInput != "quit") {
    std::cout << "To quit type 'quit' or set interpolation value: ";
    std::cin >> userInput;
    interpolationFactor = strtod(userInput.c_str(), NULL);
    if (interpolationFactor > 1) {
      interpolationFactor = 1;
    }
    if (interpolationFactor < 0) {
      interpolationFactor = 0;
    }
    
    if (userInput == "rampupslow") {
      interpolationFactor = 0;
      while (interpolationFactor < 1) {
        // Wait for one 
        double waitSeconds = hopSize/double(sampleRate);
        std::this_thread::sleep_for(std::chrono::milliseconds(int(1000*waitSeconds)));
        // add
        interpolationFactor += 0.001; 
      }
      interpolationFactor = 1;
    }

    if (userInput == "rampupfast") {
      interpolationFactor = 0;
      while (interpolationFactor < 1) {
        // Wait for one 
        double waitSeconds = hopSize/double(sampleRate);
        std::this_thread::sleep_for(std::chrono::milliseconds(int(1000*waitSeconds)));
        // add
        interpolationFactor += 0.01; 
      }
      interpolationFactor = 1;
    }

    if (userInput == "rampdownslow") {
      interpolationFactor = 1;
      while (interpolationFactor > 0) {
        // Wait for one 
        double waitSeconds = hopSize/double(sampleRate);
        std::this_thread::sleep_for(std::chrono::milliseconds(int(1000*waitSeconds)));
        // add
        interpolationFactor -= 0.001; 
      }
      interpolationFactor = 0;
    }

    if (userInput == "rampdownfast") {
      interpolationFactor = 1;
      while (interpolationFactor > 0) {
        // Wait for one 
        double waitSeconds = hopSize/double(sampleRate);
        std::this_thread::sleep_for(std::chrono::milliseconds(int(1000*waitSeconds)));
        // add
        interpolationFactor -= 0.01; 
      }
      interpolationFactor = 0;
    }
  }
}