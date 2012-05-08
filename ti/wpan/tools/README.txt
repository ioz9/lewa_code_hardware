Tools folder contains app or utilities for testing BT/FM

1. BlutoothSCOApp -
    Description: The app gets built as apk and can be launched from App menu. The app  establishes a SCO connection
    for non-voice call use case, for eg. Music streaming(mono) on BT SCO link. Also, BT SCO voice record  can also
    be tested from sound recorder after running this app.
    NOTE: The app is made to work for OMAP4 platform. OMAP4 has dedicated support for MM playback on BT SCO
          so just establish SCO connection and play music in media player. OMAP4 ABE takes care of 44.1 to 8k conversion.

          For other platform or omap3, the  user needs to play mono 8k sample using aplay on shell

2. kfmapp -
    Description: This is a test tool to exercise function of v4l2 exposed by the FM V4L2 driver above the
    TI-ST driver. This expects the /dev/radio0 to exist in the system - which is when FM V4L2 driver is
    insmod-ed.
    Generic functionalities like set frequency, FM volume, RDS enable/disable and get are all expected
    to work straight-away.
    Currently works on OMAP4, but there is no reason why it shouldn't work on OMAP3 platform as well provided
    the build includes the FM V4L2 drivers.
    Note: This may or may not enable the audio - depending on the state of the framework modifications in
    the system.

