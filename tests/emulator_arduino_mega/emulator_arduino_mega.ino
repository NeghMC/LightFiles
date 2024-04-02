#include <EEPROM.h>
#include "light_files.h"

// -------- driver implementation -------

static size_t sBlockSize;
static size_t sBlockCount;

lf_result_t lf_app_init(lf_memory_config *config) {
  uint16_t blockSize = 128;
  config->blockSize = sBlockSize = blockSize;
  config->blockCount = sBlockCount = EEPROM.length() / blockSize;
  return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_write(uint16_t block, uint16_t offset, void *buffer, size_t length, uint8_t flush) {
  for (size_t i = 0; i < length; ++i) {
    EEPROM.write(block * sBlockSize + offset + i, ((uint8_t *)buffer)[i]);
  }
  return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_read(uint16_t block, uint16_t offset, void *buffer, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    ((uint8_t *)buffer)[i] = EEPROM.read(block * sBlockSize + offset + i);
  }
  return LF_RESULT_SUCCESS;
}

lf_result_t lf_app_delete(uint16_t block) {
  for (size_t i = 0; i < sBlockSize; ++i) {
    EEPROM.write(block * sBlockSize + i, 0xff);
  }
  return LF_RESULT_SUCCESS;
}

// ------ Notes definition ------

struct Note {
  bool active;
  uint8_t id;
  uint8_t contentOffset;
  uint16_t contentLength;
  String title;
};

static const int MAX_NOTES = 8;
static const int MAX_TITLE_LENGTH = 256;

class NotesPoll {
  Note notes[MAX_NOTES];
public:
  NotesPoll() {
    for (int i = 0; i < MAX_NOTES; ++i) {
      notes[i].active = false;
      notes[i].id = i;
    }
  }

  Note *allocate() {
    uint8_t i;
    for (i = 0; i < MAX_NOTES; ++i) {
      if (notes[i].active == false) {
        notes[i].active = true;
        return notes + i;
      }
    }
    return NULL;
  }

  Note *find(String title) {
    for (int i = 0; i < MAX_NOTES; ++i) {
      if (notes[i].active && notes[i].title == title) {
        return notes + i;
      }
    }
    return NULL;
  }

  void remove(Note *note) {
    note->active = false;
  }

  Note *get_notes() {
    return notes;
  }
};

static NotesPoll notes;

static lf_result_t result;

const bool FIRST_USE = true;

// ------ Main appliaction ------

void error_handler(String msg) {
  Serial.println(msg);
  while (1)
    ;
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(0xffffffff);
  lf_init();

  Serial.println("\nLightNotepad 9000. To see commands type 'help'.");

  if (FIRST_USE) {
    Serial.println("Preparing for the first use...");
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0xff);
    }
    Serial.println("Done.");
  } else {
    Serial.println("Caching exising notes...");
    for (uint8_t i = 0; i < MAX_NOTES; i++) {
      result = lf_open(i);
      switch (result) {
        case LF_RESULT_SUCCESS:
          uint8_t header[3];
          result = lf_read(&header, 3);
          if (result != LF_RESULT_SUCCESS) {
            error_handler("Could not read title length for file " + String(i) + ".");
          }
          uint8_t titleLength = header[0];
          char titleBuffer[MAX_TITLE_LENGTH + 1];
          result = lf_read(titleBuffer, titleLength);
          if (result != LF_RESULT_SUCCESS) {
            error_handler("Could not read title for file " + String(i) + ".");
          }
          titleBuffer[titleLength] = '\0';
          result = lf_close();
          if (result != LF_RESULT_SUCCESS) {
            error_handler("Could not close file " + String(i) + ".");
          }
          Note *note = &notes.get_notes()[i];
          note->active = true;
          note->contentLength = *((uint16_t *)(header + 1));
          note->contentOffset = 3 + titleLength;
          note->title = String(titleBuffer);
          break;
        case LF_RESULT_NOT_EXISTS:
          break;
        default:
          error_handler("Could not open file " + String(i) + ".");
      }
    }
    Serial.println("Done.");
  }
}

void loop() {
  Serial.println("\n>");
  String input = Serial.readStringUntil('\n');
  String cmd = "", title = "", content = "";
  do {
    // parse cmd
    int spaceIndex = input.indexOf(' ');
    if (spaceIndex != -1) {  // If a space is found
      cmd = input.substring(0, spaceIndex);
      input = input.substring(spaceIndex + 1);
    } else {
      cmd = input;
      break;
    }
    // parse arg
    spaceIndex = input.indexOf(' ');
    if (spaceIndex != -1) {  // If a space is found
      title = input.substring(0, spaceIndex);
      content = input.substring(spaceIndex + 1);
    } else {
      title = input;
      break;
    }
  } while (0);

  if (cmd == "help") {
    Serial.println("'help' - displays available commands");
    Serial.println("'titles' - prints all the titles of the notes");
    Serial.println("'read [title]' - prints a note with a specific title");
    Serial.println("'write [title] [content]' - create a note with a specific title and content");
    Serial.println("'delete [title]' - delete specific note");
  } else if (cmd == "titles") {
    Note *notesArray = notes.get_notes();
    for (int i = 0; i < MAX_NOTES; ++i) {
      if (notesArray[i].active) {
        Serial.print("'" + notesArray[i].title + "' ");
      }
    }
    Serial.println("");
  } else if (cmd == "read") {
    do {
      Note *note = notes.find(title);
      if (note == NULL) {
        Serial.println("Error: title not found!");
        break;
      }
      result = lf_open(note->id);
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not open note " + String(note->id) + ".");
      }
      // dummy read
      result = lf_read(NULL, note->contentOffset);
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not skip data in note note " + String(note->id) + ".");
      }
      const int READ_BUFFER_SIZE = 128;
      char buffer[READ_BUFFER_SIZE];
      uint16_t noteLength = note->contentLength;
      do {
        uint16_t toRead = (noteLength > READ_BUFFER_SIZE) ? READ_BUFFER_SIZE : noteLength;
        result = lf_read(buffer, toRead);
        if (result != LF_RESULT_SUCCESS) {
          error_handler("Could not read note " + String(note->id) + ".");
        }
        Serial.write(buffer, toRead);
        noteLength -= toRead;
      } while (noteLength > 0);
      result = lf_close();
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not close note " + String(note->id) + ".");
      }
    } while (0);
  } else if (cmd == "write") {
    do {
      Note *note = notes.find(title);
      if (note != NULL) {
        Serial.println("Error: title already exists!");
        break;
      }
      note = notes.allocate();
      if (note == NULL) {
        Serial.println("Error: cant add more notes!");
        break;
      }
      result = lf_create(note->id);
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not create note " + String(note->id) + ".");
      }
      uint8_t header[3] = { title.length() };
      *((uint16_t *)(header + 1)) = content.length();
      result = lf_write(header, 3);
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not write note " + String(note->id) + ".");
      }
      result = lf_write(title.c_str(), title.length());
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not write note " + String(note->id) + ".");
      }
      result = lf_write(content.c_str(), content.length());
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not write note " + String(note->id) + ".");
      }
      result = lf_save();
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not save note " + String(note->id) + ".");
      }
      note->title = title;
      note->contentLength = content.length();
      note->contentOffset = 3 + title.length();
      Serial.println("Note saved.");
    } while (0);
  } else if (cmd == "delete") {
    do {
      Note *note = notes.find(title);
      if (note == NULL) {
        Serial.println("Error: title not found!");
        break;
      }
      result = lf_delete(note->id);
      if (result != LF_RESULT_SUCCESS) {
        error_handler("Could not delete note " + String(note->id) + ".");
      }
      notes.remove(note);
      Serial.println("Note deleted.");
      break;
    } while (0);
  } else {
    Serial.println("Unknown command " + cmd);
  }
}
