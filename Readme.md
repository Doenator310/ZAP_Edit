## ZAP_Edit

Edit .Zap files of the game "thrillville: off the rails". 

This tool enables you to edit or extract ZAP files.
Make sure that you make a backup of the files that you want to edit.


# USAGE

Open a ZAP file with the icon 

<img src="readme_data/image-0.png" alt="Alt-Text" width="600" />
<div style="display: flex;">
  <img align="left" src="readme_data/red.png" alt="Alt-Text" width="20" height="20" 
  style="margin-right: 10px;"/>
  <p>Open a ZAP File</p>
</div>

<div style="display: inline-block;">
<img align="left" src="readme_data/cyan.png" alt="Alt-Text" width="20" height="20" 
  style="margin-right: 10px;"/>
  <p>Save the changes as new ZAP file</p>
</div>

<div style="display: inline-block;">
<img align="left" src="readme_data/orange.png" alt="Alt-Text" width="20" height="20" 
  style="margin-right: 10px;"/>
  <p>Go to parent folder</p>
</div>


## How to change anything

This tool is like a file explorer but with some special functions. Basic functionalities like Replace, Delete or rename are supported. 

If you want to add a new file, just drag and drop it in here. (You cannot drag and drop folders)

<img src="readme_data/image-2.png" alt="Alt-Text" width="600" />

Folders have a similar context-menu:

<img src="readme_data/image-3.png" alt="Alt-Text" width="200" />

Sometimes Audio/OGG files are embedded in the ZAP file. These (Orange Highlighted) files can be extracted (with the Special-Extractor) OR edited in a dedicated GUI screen by clicking Edit-Audio.

<img src="readme_data/image.png" alt="Alt-Text" width="400" />

# Audio Editing
<img src="readme_data/image-4.png" alt="Alt-Text" width="600" />

<div style="display: flex;">
  <img align="left" src="readme_data/orange.png" alt="Alt-Text" width="20" height="20" 
  style="margin-right: 10px;"/>
  <p>Here are the embedded tracks, double left click on a track-row to play it</p>
</div>

<div style="display: inline-block;">
  <img align="left" src="readme_data/cyan.png" alt="Alt-Text" width="20" height="20" 
  style="margin-right: 10px;"/>
  <p>Click here to apply changes</p>
</div>

<div style="display: flex;">
  <img align="left" src="readme_data/green.png" alt="Alt-Text" width="20" height="20" 
  style="margin-right: 10px;"/>
  <p>Your typical audio player</p>
</div>

<div style="display: flex;">
  <img align="left" src="readme_data/yellow.png" alt="Alt-Text" width="20" height="20" 
  style="margin-right: 10px;"/>
  <p>Leave the Audio-Editor. Throws away unsaved audio.</p>
</div>


## What type must my audio have to work?
It must be an OGG File.

Make sure that your replacement audio file has similar characteristics to the file that you will replace. 
Depending on the situation the length can be 
relevant.

### Examples:
Entertainer audio:

- Channels: Stereo
- Sampling Rate: 44100 Hz <-- This is very important
- Bits per Sample: 32
- Bitrate: 80 kB/s

Park intro voice:
- Channels: Stereo
- Sampling Rate: 22050 Hz <-- This is very important
- Bits per Sample: 32
- Bitrate: 30 kB/s

Park Radio:
- Channels: Stereo
- Sampling Rate: 32000 Hz <-- This is very important
- Bits per Sample: 32
- Bitrate: 70 kB/s

I think the bitrate of the ogg file can be higher. 

## Limitations
- Only Thrillville off the rails [PC] is supported
- Currently you cannot create new folders in the zap file


# Credits
I wanted to merge my Zap editor with an audio editing tool and include a user interface. I was inspired to create this program by the following tool:
https://github.com/ralf2oo2/ThrillEdit

This was made with the QtFramework Version 6.5
https://www.qt.io/download-open-source
