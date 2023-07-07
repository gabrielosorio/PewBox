// Internal MIDI Handlers
// Relies on the Arduino MIDI Library
// Interfaces with the Sequencer

// Read and handle incoming MIDI messages
// Put this in the run loop
void readMidi() {
  if (MIDI.read()) {
    if (MIDI.getType() != 248) {
      Serial.println(MIDI.getType());
      Serial.print("Data1: ");
      Serial.println(MIDI.getData1());
      Serial.print("Data2: ");
      Serial.println(MIDI.getData2());
    }

    switch (MIDI.getType()) {
      case midi::Start:
        Serial.println("Start");
        sequencerPlay();
        break;
      case midi::Continue:
        Serial.println("Continue");
        sequencerPlay();
        break;
      case midi::Stop:
        Serial.println("Stop");
        sequencerPause();
        break;
      case midi::ControlChange:
        if (MIDI.getData1() == 123) {
          Serial.println("[Channel Mode Message] All Notes Off");
          sequencerPause();
        }
        break;
      case midi::NoteOn:
        Serial.println("Note On");
        break;
      case midi::NoteOff:
        Serial.println("Note Off");
        break;
      case midi::Clock:
        externalClockHandler();
        break;
      default:
        break;
    }
  }
}
