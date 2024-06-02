# Autosteer_GPS_Nano

Ce code permet de connecter votre récepteur GNSS au module Autosteer basé sur Arduino Nano. Vous pouvez donc profiter d'un AIO avec un vieux PCBv2. C'était plus de la curiosité mais cela fonctionne alors voici le code...

PANDA fonctionne, vous pouvez l'utiliser (par defaut) ou sinon vous pouvez utiliser sans et le code envois seulement NTRIP et recevoir NMEA au recepteur GNSS via le module Autosteer UDP.

This code connects your GNSS receiver to the Arduino Nano based Autosteer module. So you can enjoy an AIO with an old PCBv2. It was more curiosity but it works so here is the code...

PANDA works, you can use it (default) or alternatively you can use without and the code sends only NTRIP and receive NMEA to the GNSS receiver via the Autosteer UDP module.

Just need uploaded to Nano. set the receiver to 115200Bauds with GGA and VTG 10hz.

# Attention!!!

Attention!!! pour télécharger sur le nano, le récepteur doit être débranché.
Attention!!! to upload to the nano the receiver must be unplugged.

# Hardware

simple, simply connect TX and RX of the Nano to those of the receiver.

![Panda](Dieren.jpg)



