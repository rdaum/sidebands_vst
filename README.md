# sidebands
This is a VST3 synthesizer combining additive and a form of Modified FM synthesis.

Specifically the FM synthesis is based on the maths described by Victor Lazzarini and Joseph Timoney in their paper
@ https://mural.maynoothuniversity.ie/4697/1/JAES_V58_6_PG459hirez.pdf. Or at least as best as I understood it.

This form of FM produces a sound less 'nasal' and potentially more organic sounding than traditional "Yamaha DX" style
FM/PM.

![screenshot](screenshot.png?raw=true)

It is composed of a number of (configurable in source) FM "generators", each with their own 7 stage envelope and/or LFO.
Combining each of these generators together additively can generate nice complex timbres. 

I originally intended to make this into a for-pay product, but I have not had time to complete it, so I'm making it open source.

Some features (for example LFO) remain unimplemented in the UI but are actually implemented in the synthesizer engine. I will continue to add features, and hopefully publish binaries soon.

I will continue to add features as I find the time.

One novel and useful thing about this VST that other developers might find interesting is that the UI uses  
an embedded webview instead of the Steinberg's "VSTGUI" framework. This allowed me to write a crossplatform UI
in TypeScript/JavaScript/HTML/CSS. At some point I will split the pieces under controller/webview out into their own
repository and offer this as an open source package for other developers interested in doing this. 

It runs on Windows and Linux, the work to port to the Mac has not been done yet as I currently lack a Mac to test with.

In particular, to port to OS X:

  * The work to use WebKit on Mac would need to be done, likely starting from the Linux WebKit2 implementation.
  * for M1 Macs, there are AVX512 x86 dependencies at the moment that would have to be ported to ARM NEON
  




