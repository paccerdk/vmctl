# vmctl
Command line interface for VoiceMeeter API requests

API documentation can be found [here](https://github.com/vburel2018/Voicemeeter-SDK/blob/main/VoicemeeterRemoteAPI.pdf)

  #### Usage:
vmctl.exe [options] [API call] [--]
  
  
###### Options:
  
    -v, --verbose			Verbose mode: Useful for debugging and seeing what's going on
    --, -i, --interactive		Interactive mode: Read commands from standard input until it's closed
    -h, --help			Show this help
    
###### Examples:
     vmctl.exe Strip[0].Gain=-1		Set the gain of the first strip to -1
     vmctl.exe Strip[0].Label		Get the label of the first strip
     vmctl.exe -v Strip[0].Label=MyStrip	Set the label of the first strip to MyStrip, and show verbose output
    

###### Interactive mode Commands:
    exit, quit			Exit the program
    help				Show this help
    verbose				Toggle verbose mode
    debug				Toggle debug mode
    interactive			Enter interactive mode
    <API call>[=value]		Get/Set the specified API entry\n

###### Examples:
     Strip[0].Gain=-1		  Set the gain of the first strip to -1
     Strip[0].Label			  Get the label of the first strip
     Strip[0].Label=MyStrip		  Set the label of the first strip to MyStrip
