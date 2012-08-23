-------------------------------------------
  * ontheflY - an OpenGL based techdemo *
-------------------------------------------
Copyright (C) 2003 Nicola Orrù http://www.itadinanta.it

Last revision: Monday, November 17 2003

This software is distributed under the GPL, see LICENSE.txt and source
code for details.

* Linux Requirements *

You need the Mesa GL library runtime and the GLU runtime shared
library installed on your system. And, of course, XFree or
equivalent. A supported accelerated 3d card is highly recommended to
run the demo at a reasonable pace.

* Windows Requirements *

opengl32.dll and glut32.dll libraries should be already present on
your system but you still need the glut32.dll. Fortunately, the
library is redistributed here for your convenience, according to:

http://www.opengl.org/developers/documentation/glut/#windows

If your sistem lacks the OpenGL dlls, you can download them from that
site, too.

* Linux instructions *

Unpack the zipfile onto a directory of your choice, enter it, then
type

   ./onthefly

to start the demo inside a 640x480 window.

   ./onthefly -fullscreen

starts the demo full screen at the current resolution. For a good
framerate at higher resolutions, you should have a quite powerful
system. You may want to decrease your screen resolution (i.e. by
running "xrandr" if your XFree installation supports that extension)
to obtain a smoother animation.

   ./onthefly -interactive

starts the demo in interactive mode.

* Windows instructions *

Unpack the zipfile onto a new folder, open the folder, then click on

    ontheflywin.exe

to start.

If you want the demo to run fullscreen, you can click on 

    onthefly-fullscreen

It runs full screen at the current resolution, so if you haven't got a
powerful system but you want to achieve a good framerate, you may
choose a lower resolution before starting the demo.

Running

    ontheflywin.exe -interactive

from a cmd window, starts the demo in interactive mode.

Usage
-----

Once installed and started, there is no difference between the Linux
and the Windows versions.

You can start the demo in automatic mode and switch to interactive
mode at any minute during the simulation, simply pressing the arrow
keys.

You can press the "h" key to show an onscreen help frame. "f" show
animation statistics.


How the simulation works
------------------------

The real simulation is based upon a simple artificial three-level
ecosystem, based on the following rules: 

- there are several types of particles:

* the Light, at the center of the scene
* The Flies, you can split into three sets:
  . the Preys, that appear as small lamps with a little tail
  . the Predators, that look like a sylized mix between fireflies and dragonflies
  . the Queen, that is like a big Predator
* the Generators (that emit fire from six spots)
* the eYe candies, that do not interact with anything, they're simply neat

- the Light, acting as a primary energy source, can feed the Preys,
that drain a certain amount of energy from it during each frame. The
amount of energy drained depends on the distance from the light: it is
zero near the light, maximum at a certain distance, then decreases
gradually. By draining energy from the light, the Preys can grow and
extend their lifetime.

- if a Prey drains energy from the Light, the light becomes
smaller. When the Light reaches its minimum size, it cannot feed Preys
anymore. Besides, if not used, the Light grows at a constant pace up
to its maximum size.

- if a Prey stays too near or too far from the light, it loses
"weight". If a Prey becomes too small, it can't mate anymore.

- Flies interact from small distances, colliding or simply thrust away
each other through a "repulsing force field".

- if a Predator collides with a smaller prey, eats it, thus growing
and extending its lifetime. If a predator makes contact with a larger
Prey, they simple bounce onto each other. A Predator must eat Preys in
order to stay alive.

- Bigger Flies move slower

- When a particle finishes its lifetime (TTL), it dies and disappears
from the scene.

- once a Fly reaches a threshold size, it is ready for
mating. Predators can mate with other Predators or with the
Queen. Preys mate wit other Preys only.

- When a Fly reproduces itself after mating, it splits into two
smaller particles one half the original size. The "mother" simply
keeps going, while the "child" obtains a "genetic code" (a string of
bits containing some flight-direction parameters) that is a mix
between its parents ones. The "father" is not affected by this
operation. In a mating session, a Fly can be both a "father" and a
"mother".

- One bit of the "offspring"'s genetic code may be toggled randomly
(mutation). When it happens, the child changes color. You can see if a
mutation is advantageous simply looking at the color of the resulting
swarm after some generations.  The mutation will be inherited by
further generations.

- Only the strongest can survive
