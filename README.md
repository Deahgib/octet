

Rope Bridge
-----------

Tools & Middleware, Assignment 1. By, **Louis Bennette**.  Id: lbenn004


Video: https://youtu.be/0lu2ozDIwPk


**The bridge:**

I have created a rope bridge using primitive meshes and bullet3 constraints. 
Specifically, I tested my bridge using the btGeneric6DofSpringConstraint and the btHingeConstraint object provided by bullet3. I learnt how to implement these thanks to this online document http://bulletphysics.org/mediawiki-1.5.8/index.php/Constraints 


I started to make a bridge using thin rectangle boxes as planks. In order for the bridge to be held up and hang appropriately, I created two static rigid bodies to act as anchors for both ends of the bridge.


**Springs:**

A spring constraint requires two rigid bodies and two points where the ends of the spring attach to the rigid bodies. Here the rigidbodies are the planks and the anchors. And the points are btVector3 objects representing a local point in each rigidbody.
See, http://bulletphysics.org/Bullet/BulletFull/classbtGeneric6DofSpringConstraint.html 


The bridge was constructed by stringing the planks all together using spring constraints and also tieing the first and last plank to the left and right anchor respectively.
Each plank is connected to it’s neighbour using two spring constraints on either end of the plank. Without two connections between each plank the planks would start to rotate about the spring pivot point. Two springs on either end of the plank is more stable and of course similar to typical rope bridges.
If the planks are neatly lined up next to each other they will perfectly fit the span between the two anchors.
I want the planks to sag so I use `setLinearUpperLimit()` with a small value for the springs to have a little distance in which they are allowed to move. This creates a small gap between each plank. The bridge droops a little, giving it it’s typical rope bridge characteristic.

Note, the spring can stretch further as it’s a soft constraint meaning that it can deform and veer out of its range if necessary. Therefore outside forces and objects often demonstrate this especially when very heavy objects are resting on the bridge, sometimes deforming it beyond recognition.


Below is a representation of the springs (in blue) between the planks of the bridge.
![Highlighted Spring Constraints](https://raw.githubusercontent.com/Deahgib/octet/rope-bridge/octet/assets/rope-bridge/rope-bridge-springs.png)

**Hinges:**

To demonstrate how hinge constraints could be used, I added decorations in the form of small spheres or baubles on the bridge.
I used the btHingeConstraint to tie the baubles to the planks . There are two types of hinges. The simple of the two requires a rigid body, a pivot point and an axis along which the rotation will happen. This isn’t useful for our purposes but was a good exercise to test. (Implemented in an earlier commit)
The other type of hinge requires, much like the spring constraint, two rigidbodies, local points for the pivot in each rigid body and additionally two axes around which the pivot will occur.
See, http://bulletphysics.org/Bullet/BulletFull/classbtHingeConstraint.html 


I made the baubles pivot around a point beneath the plank and allowed rotation along all three axes. This makes the hinge behave similarly to a ball joint. When any force is applied to a plank the baubles attached to it will move and sway unpredictably. This means the bridge takes longer to reach a fully rested state.
Each plank has two baubles attached to it one on each side of the plank for even weight distribution.
Note, the hinges are also soft constraints and can be stretched aswell by outside forces. 


Below is an image of where the hinge point is (in blue) for a single bauble.

![Highlighted hinge constraint](https://raw.githubusercontent.com/Deahgib/octet/rope-bridge/octet/assets/rope-bridge/rope-bridge-hinges.png)


**Drop test:**

To test if the bridge reacts properly to other objects on top of it, I simply drop a sphere onto the bridge when the user presses ‘space’. The sphere lands on one end of the bridge and the geometry of the bridge makes the ball roll all the way across. 




I’ve also used hinge constraints between the planks to test how hinge constraints behave compared to spring constraints. The bridge using hinge constraint behaves mostly the same. It was less elastic than the spring bridge. However, I could only make the hinges pivot around one axis. This meant there was no roll in the planks even when a force was acting on the planks.


The user can set the weights of the scene objects by editing the ‘masses.csv’ file.
The default file is as follows: `0.5,0.05,1`
These three values are used to set the weights of each plank, each bauble and each dropping ball respectively. If the user wishes to test the elasticity of the bridge they can make the dropping ball weight higher.

***Basic overview of how bullet3 works in octet:***

If bullet3 is enabled for an octet app the physical world is set up when when a visual_scene object is initialised.
The following bullet code is executed (Taken from the constructor in visual_scene.h):
    

    dispatcher = new btCollisionDispatcher(&config);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);


This creates for us, an object to handle collisions(dispatcher) and an object to calculate collisions between two objects (broadphase).
More importantly for us it adds a constraint solver which is responsible for calculating what will happen to the hinges and springs in my scene.
And moreover, it adds a world object which holds all of the relevant data used to calculate the rigidbodies positions based on the current forces, constraints or collisions in play for each object. The world object also holds data for values like gravity, air density, time, etc.


When a primitive shape is added to the scene and bullet3 is enabled a rigidbody is generated automatically using the mesh of the created shape. 
The rigidbody object can be accessed through the scene node object for each mesh_instance in the scene. 
We can use these rigidbodies to create constraints.
For this project I altered the code in visual scene to add the  `addHingeConstraint()` and `addSpringConstraint()`  functions. I had to do this in visual_scene to have access to the world object to call the 
`world->addConstraint(myConstraint)` function which is what adds the constraint to the world which in turn will call the constraint solver for any constraint based calculations. Another, perhaps better, approach would be to extend the visual_scene class and add these extra functions as well as all of our meshes, springs, and hinges for better access.


NB: I separated the rope_bridge code out into it’s own class. Here this serves no real purpose. I was originally planning on making it possible to create a bridge based on location parameters. This was slightly too ambitious as I didn’t anticipate the enormous number of parameters required to make a bridge and ran out of time. 



> Written with [StackEdit](https://stackedit.io/).

------------

# octet

Octet is a framework for teaching OpenGL and the rudiments of game programming such
as Geometry construction, Shaders, Matrices, Rigid body Physics and Fluid dynamics.

It has a number of examples in the src/examples directory.

To use with visual studio, fork this repository into your own account and then
"Clone Into Desktop" using the GitHub tool and open one of the .sln files in src/examples.

There is a python script for generating your own projects from a template.

From the octet directory run:

packaging\make_example.py my_example

To create your own project in src/examples

Examples should also work with Xcode, although testing is a lot less thorough. If it does not work, send
me a pull request with fixes, please...

Octet is a bit unusual in that it does not use external libraries such as libjpeg or zlib.
These are implemented in source form in the framework so that you can understand the code.
The source of most academic libraries is almost unreadble, so we aim to help your understanding
of coding codecs such as GIF, JPEG, ZIP and so on.