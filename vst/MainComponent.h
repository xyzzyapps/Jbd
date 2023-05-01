#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>


#include <exception>
#include <typeinfo>
#include <stdexcept>
#include <vector>
#include <regex>
#include <chaiscript/chaiscript.hpp>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::filesystem::current_path;
using namespace std;

std::vector<std::string> splitString(const std::string& str, std::string sep)
{
    std::vector<std::string> tokens;
    
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// https://github.com/adamski/RestRequest/blob/master/Source/RestRequest.h


class RestRequest
{
public:

    RestRequest (String urlString) : url (urlString) {}
    RestRequest (URL url)          : url (url) {}
    RestRequest () {}

    struct Response
    {
        Result result;
        StringPairArray headers;
        var body;
        String bodyAsString;
        int status;

        Response() : result (Result::ok()), status (0) {} // not sure about using Result if we have to initialise it to ok...
    } response;


    RestRequest::Response execute ()
    {
        auto urlRequest = url.getChildURL (endpoint);
        bool hasFields = (fields.getProperties().size() > 0);
        if (hasFields)
        {
            MemoryOutputStream output;

            fields.writeAsJSON (output, 0, false, 20);
            urlRequest = urlRequest.withPOSTData (output.toString());
        }

        std::unique_ptr<InputStream> input (urlRequest.createInputStream (hasFields, nullptr, nullptr, stringPairArrayToHeaderString(headers), 0, &response.headers, &response.status, 5, verb));

        response.result = checkInputStream (input);
        if (response.result.failed()) return response;

        response.bodyAsString = input->readEntireStreamAsString();
        response.result = JSON::parse(response.bodyAsString, response.body);

        return response;
    }


    RestRequest get (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "GET";
        req.endpoint = endpoint;

        return req;
    }

    RestRequest post (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "POST";
        req.endpoint = endpoint;

        return req;
    }

    RestRequest put (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "PUT";
        req.endpoint = endpoint;

        return req;
    }

    RestRequest del (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "DELETE";
        req.endpoint = endpoint;

        return req;
    }

    RestRequest field (const String& name, const var& value)
    {
        fields.setProperty(name, value);
        return *this;
    }

    RestRequest header (const String& name, const String& value)
    {
        RestRequest req (*this);
        headers.set (name, value);
        return req;
    }

    const URL& getURL() const
    {
        return url;
    }

    const String& getBodyAsString() const
    {
        return bodyAsString;
    }

private:
    URL url;
    StringPairArray headers;
    String verb;
    String endpoint;
    DynamicObject fields;
    String bodyAsString;

    Result checkInputStream (std::unique_ptr<InputStream>& input)
    {
        if (! input) return Result::fail ("HTTP request failed, check your internet connection");
        return Result::ok();
    }

    static String stringPairArrayToHeaderString(StringPairArray stringPairArray)
    {
        String result;
        for (auto key : stringPairArray.getAllKeys())
        {
            result += key + ": " + stringPairArray.getValue(key, "") + "\n";
        }
        return result;
    }
};



class MainContentComponent   : public juce::AudioProcessor
{
public:

    MainContentComponent()
    : AudioProcessor (BusesProperties())
    {

     
    }


    ~MainContentComponent() override
    {
    }

    void prepareToPlay	(double sampleRate,int maximumExpectedSamplesPerBlock)	 override
    {
    }

    long timeToSamples(double time) {
        return long(time * getSampleRate());
    }

    void releaseResources() override {}

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        jassert (buffer.getNumChannels() == 0);                                                         
        auto numSamples = buffer.getNumSamples();                                                       
        buffer.clear();
        
        AudioPlayHead::CurrentPositionInfo thePositionInfo;
        juce::AudioPlayHead *playHead = getPlayHead();
	
		if (playHead == nullptr) return;

        playHead->getCurrentPosition(thePositionInfo);

        auto currentPosition = thePositionInfo.timeInSeconds;              


       if (thePositionInfo.isPlaying) {
            sampleCount += numSamples;

            if (loopStart == -1) {
                loopStart = currentPosition;
                return;
            } 

            if (currentPosition == loopStart) {
                loopLength = loopEndTime - loopStart;
                sampleCount = 0;
            }

            loopEndTime = currentPosition;

       }    

        if (thePositionInfo.isPlaying && !midiLoopDone) {

            if ((currentPosition == loopStart) && !processedMidi.isEmpty()) {
                midiLoopDone = true;
                return;
            }

            if (midiLoopDone == false) {

                for (const auto metadata : midi)
                {
                    
                    auto message = metadata.getMessage();
                    auto time = metadata.samplePosition;

                    if (message.isNoteOn())
                    {
                        auto message2 = juce::MidiMessage::noteOn(message.getChannel(),
                           message.getNoteNumber(),
                           message.getVelocity());
                        processedMidi.addEvent(message2, 0);
                        noteTimeStamps.push_back(currentPosition - loopStart);
                    }
                }
            }
        }


        if (!thePositionInfo.isPlaying) {
            sampleCount = 0;
        }
        

        if (thePositionInfo.isPlaying && dataInferred) {
           
           bufferStartTime = sampleCount;
           bufferEndTime = bufferStartTime + numSamples; 

           for (const auto metadata : inferMidi)  {
               
            auto message = metadata.getMessage();
            auto time = metadata.samplePosition;

            if ((bufferStartTime <= time) && (time <= bufferEndTime)) {

                uint8 vol = 56;
                if (message.isNoteOn()) {
                    auto message2 = juce::MidiMessage::noteOn(
                        message.getChannel(),
                        message.getNoteNumber(),
                        vol
                    );
                    message2.setTimeStamp(message.getTimeStamp());
                    inferProcessedMidi.addEvent(message2, message.getTimeStamp());
                }

    
            }

       }


        midi.swapWith (inferProcessedMidi);
    }

}

bool isMidiEffect() const override                           { return true; }

juce::AudioProcessorEditor* createEditor() override          { return new PluginAudioProcessorEditor(*this); }
bool hasEditor() const override                              { return true; }

const juce::String getName() const override                  { return "Jbd"; }

bool acceptsMidi() const override                            { return true; }
bool producesMidi() const override                           { return true; }
double getTailLengthSeconds() const override                 { return 0; }

int getNumPrograms() override                                { return 1; }
int getCurrentProgram() override                             { return 0; }
void setCurrentProgram (int) override                        {}
const juce::String getProgramName (int) override             { return {}; }
void changeProgramName (int, const juce::String&) override   {}

void getStateInformation (juce::MemoryBlock& destData) override
{
}

void setStateInformation (const void* data, int sizeInBytes) override
{
}


private:

    juce::MidiBuffer processedMidi;
    juce::MidiBuffer inferMidi;
    juce::MidiBuffer inferProcessedMidi;

    bool dataInferred = false;
    float loopStart = -1;
    float loopEndTime = -1;
    float loopLength = -1;
    std::vector <float> noteTimeStamps;
    std::vector <float> inferredNoteTimeStamps;
    bool midiLoopDone = false;
    long sampleCount = 0;
    long bufferStartTime = 0;
    long bufferEndTime = 0;


    class PluginAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Button::Listener,
    public juce::FilenameComponentListener,
    private juce::Timer
    {
    public:

        PluginAudioProcessorEditor(MainContentComponent& p)
        : AudioProcessorEditor (&p), audioProcessor (p) 
        {


         fileComp.reset (new juce::FilenameComponent ("fileComp",
                                                         {},                       // current file
                                                         false,                    // can edit file name,
                                                         false,                    // is directory,
                                                         false,                    // is for saving,
                                                         {},                       // browser wildcard suffix,
                                                         {},                       // enforced suffix,
                                                         "Select file to open"));  // text when nothing selected
         addAndMakeVisible (fileComp.get());

         fileComp->addListener (this);

         textContent.reset (new juce::TextEditor());
         addAndMakeVisible (textContent.get());
         textContent->setMultiLine (true);
         textContent->setReadOnly (true);
         textContent->setCaretVisible (false);

         addAndMakeVisible (inferButton);
         inferButton.setButtonText ("Infer");
         inferButton.addListener (this);

         addAndMakeVisible (resetButton);
         resetButton.setButtonText ("Reset Loop");
         resetButton.addListener (this);

         addAndMakeVisible (testMidiGenerateButton);
         testMidiGenerateButton.setButtonText ("Test Midi");
         testMidiGenerateButton.addListener (this);

         chai.add(chaiscript::fun(&PluginAudioProcessorEditor::testMidi), "testMidi");


         setSize (600, 400);
         startTimer (400);
     }

     void testMidi() {
        auto text = textContent->getText();
        textContent->setText(text + "\nfrom chai");

    }

    ~PluginAudioProcessorEditor() {
    }


    void paint (juce::Graphics& g)
    {
        g.fillAll (juce::Colours::white);
        g.setColour (juce::Colours::black);
        g.setFont (15.0f);
        g.drawFittedText ("Midi Volume", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
    }

    void resized() override
    {
        fileComp.get()->setBounds (10, 10, getWidth() - 20, 20);
        textContent.get()->setBounds (10, 40, getWidth() - 20, 120);
        inferButton.setBounds (10, 180, getWidth() - 20, 20);
        resetButton.setBounds (10, 200, getWidth() - 20, 20);
        testMidiGenerateButton.setBounds (10, 300, getWidth() - 20, 20);
    }


private:
    std::unique_ptr<juce::FilenameComponent> fileComp;
    std::unique_ptr<juce::TextEditor> textContent;
    juce::TextButton inferButton;
    juce::TextButton resetButton;
    juce::TextButton testMidiGenerateButton;;

    chaiscript::ChaiScript chai;

    MainContentComponent& audioProcessor;
    
    void timerCallback() override
    {
        if (audioProcessor.dataInferred != true) {

            std::string text = "";

            if (audioProcessor.midiLoopDone == true) {
                text += "Sample Count ..." + std::to_string(audioProcessor.sampleCount) + "\n";
                text += "loop End ..." + std::to_string(audioProcessor.loopEndTime) + "\n";
                text += "loop length ..." + std::to_string(audioProcessor.loopLength) + "\n";
                text += "midi\n";
                for (const auto metadata : audioProcessor.processedMidi)  {
                    auto message = metadata.getMessage();
                    auto time = metadata.samplePosition;

                    if (message.isNoteOn()) {
                        text += std::to_string(message.getNoteNumber()) + ";" + std::to_string(time) + "\n";
                    }
                }
            } else {
                AudioPlayHead::CurrentPositionInfo thePositionInfo;
                audioProcessor.getPlayHead()->getCurrentPosition(thePositionInfo); 
                auto currentPosition = thePositionInfo.timeInSeconds;              
                text += "Sample Count ..." + std::to_string(audioProcessor.sampleCount) + "\n";
                text += "processing ..." + std::to_string(currentPosition) + "\n";
                text += "loop start ..." + std::to_string(audioProcessor.loopStart) + "\n";
                
                if (!audioProcessor.processedMidi.isEmpty()) {
                    int counter=0;
                    for (const auto metadata : audioProcessor.processedMidi)  {
                        counter++;
                    }
                    text += "midi length ..." + std::to_string(counter);
                }
            }

            textContent->setText(text);
        } else { 
            auto text = textContent->getText().toStdString();
            text += std::to_string(audioProcessor.bufferStartTime) + "--" + std::to_string(audioProcessor.bufferEndTime) + "\n";
            textContent->setText(text);


            /*
            std::string text = "";

           for (const auto metadata : audioProcessor.inferProcessedMidi)  {
                auto message = metadata.getMessage();
                auto time = metadata.samplePosition;
                text += std::to_string(message.getNoteNumber()) + "--" + std::to_string(time) + "\n";
            }
            textContent->setText(text);
            */
        }



    }


    void filenameComponentChanged (juce::FilenameComponent* fileComponentThatHasChanged)
    {
        if (fileComponentThatHasChanged == fileComp.get()) {
            auto path = fileComp->getCurrentFile().getFullPathName().toStdString();
            
        }
    }

        void buttonClicked (juce::Button* button)   // [2]
        {
            if (button == &inferButton)                                                      // [3]
            {
             RestRequest request;
             request.header ("Content-Type", "application/json");
             std::string text = "";
             int counter=0;
             juce::MidiMessage lastMessage;

             for (const auto metadata : audioProcessor.processedMidi)  {
                auto message = metadata.getMessage();
                if (message.isNoteOn()) {
                    lastMessage = message;
                    const auto time = audioProcessor.noteTimeStamps[counter];
                    text += std::to_string(message.getNoteNumber()) + ";" + std::to_string(time) + "\n";
                    counter++;
                }
            }

            RestRequest::Response response = request.post("http://127.0.0.1:3000/infer")
            .field (juce::String("midi"), var(text))
            .field (juce::String("loopLength"), var(audioProcessor.loopLength))
            .execute();

            
            textContent->setText(response.bodyAsString);
            auto lines  = splitString(response.bodyAsString.toStdString(), "\n");
            
            for (auto const &token: lines) {
                std::vector<std::string> notes;
                std::regex rgx(";");
                std::sregex_token_iterator iter(token.begin(),
                    token.end(),
                    rgx,
                    -1);
                std::sregex_token_iterator end;
                for ( ; iter != end; ++iter)
                    notes.push_back(*iter);

                int note = std::stoi(  notes[0] );
                float time = std::stof(  notes[1] );

                audioProcessor.inferredNoteTimeStamps.push_back(time);

                auto message = juce::MidiMessage::noteOn(lastMessage.getChannel(),
                   note,
                   lastMessage.getVelocity());

                message.setTimeStamp(time * audioProcessor.getSampleRate());
                audioProcessor.inferMidi.addEvent(message,time * audioProcessor.getSampleRate());


            }
            std::string mtext = "";                    
            mtext += "new midi\n";
            for (const auto metadata : audioProcessor.inferMidi)  {
                auto message = metadata.getMessage();
                auto time = metadata.samplePosition; 

                if (message.isNoteOn()) {
                    mtext += std::to_string(message.getNoteNumber()) + "--" + std::to_string(time) + "\n";
                }

            }

            textContent->setText(mtext);

            audioProcessor.dataInferred = true;
            
        }

        if (button == &resetButton)  {
            audioProcessor.loopStart   = -1;
            audioProcessor.loopEndTime   = -1;
            audioProcessor.loopLength   = -1;

            audioProcessor.midiLoopDone    = false;
            audioProcessor.dataInferred = false;
            audioProcessor.processedMidi.clear();
            audioProcessor.inferMidi.clear();

            textContent->setText("");

        }

        if (button == &testMidiGenerateButton)  {
          chai.eval("testMidi();");
      }


  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorEditor)
};



JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


