


Tank Game
=========

 *[Escape of the Runaway Tank (working title)]*
 Intro to Game programming, Assignment 1
 by ***Louis Bennette*** , id: lbenn004
 
 Video: https://youtu.be/-xZ5i3ghtQs

I decided that I would like to turn the space invaders game into an endless scrolling tank game.
Enemies come into play from the right of the screen and make their way to the left of the screen. If an enemy makes it all the way across the screen the player will lose a life. When the player runs out of lives the game is over. When the player hits an enemy they gain some points.
The aim of the game is to rack up as many points as possible before running out of lives.
To make it more interesting the player can pick up powerups in the form of extra lives or temporary attack speed boosts.


To create the sensation of movement and scrolling along I use 2 textures for the sky and 2 textures for the terrain. The textures are moving to the left until they hit an anchor which moves the texture back to right side of the screen.Using this system the background will continue scrolling indefinitely, which is the behaviour we want. Additionally I gave the sky and terrain different speeds to create a simple parallax effect. 

There is a combo points meter awarded for continuous hits on enemies without any misses. For every consecutive enemy the player hits the combo meter is increased by 1. Each enemy awards 1 point for being killed multiplied by the combo meter. In other words if the player hits three enemies consecutively his combo meter will become X4 and his score will be 6  (1 + 2 + 3).

The game has 2 types of enemies; planes and blimps.  The planes move twice as fast as the blimps. When the enemies are above the player they will attempt to drop a bomb on the player, this is behaviour retained from the invaderers game.
The game also has 2 types of powerups (referred to as upgrades in code); a lifeup and a attack speed boost. The upgrades are activated on collision with the player and will remain active for 250 game frames.




**The CSV file level loader:**

The enemies and upgrades are spawned in based on a level.csv file.
Each line of the file represents an entity that will be spawned, the file is read once on load and the entities are stored in a std vector in the order in which they will spawn. When the end of the list is reached we loop back to the start and speed up the enemies.  
Sample file:


    10,1,0
    20,1,1
    10,2,2
    100,3,0


Each line is made up of three values which represent when, which and where an entity is spawned and by entity I’m referring to enemies and upgrades.

The first value “when” determines how many frames to wait until the next enemy is spawned. The first enemy is spawned after the first 50 frames of the game have been run this is a fixed preset value. Any subsequent spawn is determined by the value in the first column of the file. For example: after the second line entity in our sample file is spawned, the game will wait 20 frames until the third line entity is spawned.
Note there will be a longer 100 frame wait when looping back from the last entity to the first entity.


The next column represents the “which” meaning which specific entity will be spawned.
The value mapping is as follows.

 - 1 => plane (enemy)  
 - 2 => blimp (enemy)  
 - 3 => Life Up (powerup) 
 - 4 => Attack Speed + (powerup)

So for example the file above will load 2 planes, a blimp and a LifeUp upgrade in that order.


And the third column is the “where” value, representing where we want the enemy to be spawned.
The enemies can be spawned at any of 5 anchors. The anchors are indexed as 0 to 4, 0 being the topmost anchor and 4 being the lowest enemy anchor.
Note that the powerups will always spawn at their own anchor which is low down on the terrain so that that the player’s tank can pick them up. A “where” value is needed for the powerups in the file but this is only to simplify the file reading process, the value is completely ignored.




**The player tank:**


The tank is actually made up of two sprites one for the gun turret and the other for tank.
When the user moves the tank they are moving the body of the tank left and right and the gun’s x and y are being updated based on the tanks x and y every frame (View code snippet below). I don’t use set relative because the angle of the gun would be updated and this value is going to be calculated later on.

    float x, y;
    sprites[tank_sprite].get_position(cameraToWorld, x, y);
    sprites[gun_sprite].set_position(x-0.02f, y+0.2f);


The game supports a hover to aim feature. This is done by taking the screen coordinates of the cursor and translating them into scene coordinates. Using the scene coordinates I can calculate the angle of the line between the cursor and the tanks gun relative to the y axis of the scene. Using the angle I can simply rotate the tank_gun sprite by that amount for it to face the cursor. Now when the user wants to shoot I grab a missile from the missile pool and set it’s translation relative to the tanks gun. Using this system the old invaders code for moving missiles can still be used as this code just moves the missile sprite forward along its bearing.




**The fadeout effect using the fragment shader:**

I implemented a simple shader that can be “animated” all the shader does is draw any texture using a float alpha value.  The fragment shader code calculates the color of the texture for that pixel using a sampler2d and a uv coord. This color is stored in a vec4 treated as RGBA, we’re only interested in the opacity of the texture so we multiply the alpha value of the color (accessed through ‘w’)  by our uniform alpha to apply the desired opacity.
Fragment shader code:

      vec4 tex = texture2D(sampler, uv_);
      if(tex.w != 0) tex.w *= alpha;
      gl_FragColor = tex; 


In order to pass the alpha value to the shader the render function needs a new variable which in our case can default to 1. I.e. if no alpha is specified in the render just render it with alpha of 1 which will be fully opaque. 
I made a fadeout_sprite object to create the fade over time effect on a sprite. The fadeout_sprite extends the sprite class but has one key additional fade() function. 

When called, the fade function will set that sprite to become enabled, meaning it will start to render every frame, the alpha is also set to 1. In the fade out sprites render function, the alpha value is used to render the sprite and  is decreased to be used in the next render call. When the alpha value is 0 we disable the fadeout_sprite as it’s now invisible.


> Written with [StackEdit](https://stackedit.io/).


----------


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
