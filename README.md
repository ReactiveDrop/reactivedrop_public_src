# Alien Swarm: Reactive Drop #

[Alien Swarm: Reactive Drop](https://store.steampowered.com/app/563560/) is a standalone modification for Valve's Alien Swarm game. This repository contains the source code for Alien Swarm: Reactive Drop. Using Visual Studio you will be able to build these DLL files: client.dll, missionchooser.dll, server.dll.

### What is this repository for? ###
Having access to the source code you can:


* Add new weapons, NPCs, fix bugs and generally add new features to the game and submit them as pull requests
* Create mods. Both server side and client side mods. Or fully standalone mods

> Note: Mods must not be used to tamper with game's leaderboards or ranked servers.


### How to download the repository ###
There are two branches in this repository:

* __master__ branch is the original Alien Swarm source code provided by Valve
* __reactivedrop_public__ branch is the up to date Reactive Drop source code

> **Tip:** It is better to make a fork of this repository before cloning it and work on your own fork. Do this using a button in the top right. The URL for your forked repository will be different from the one shown below.

* Download and install [Git for Windows](https://git-scm.com/downloads).
* Open Git Bash
* Input commands into Git Bash
```sh
    git clone https://github.com/ReactiveDrop/reactivedrop_public_src.git
    cd reactivedrop_public_src
```

### How to compile the project ###

* Download Visual Studio 2022 Community (https://visualstudio.microsoft.com/downloads/)
* While installing Visual Studio 2022 select the 'Game development with C++' workload
* Open reactivedrop_vs13.sln using Visual Studio
* Select configuration Debug or Release
* Build
* Resulting DLLs will be placed into reactivedrop\bin

### How to modify content files ###
The reactivedrop folder contains games content: materials, models, scripts, sounds. For now it contains the bare minimum of files to save the repository space. If you want to add new or modify the existing game content files please do it in two commits:

* First commit only adds unmodified content files. You can get them from the Alien Swarm: Reactive Drop game folder
* Second commit introduces your modifications to both content files and, if necessary, source code

This way we will be able to see which changes did you introduce to the content files.

### How to submit changes ###

Changes can be submitted as pull requests.

A pull request should be as granular as possible and only change one thing at a time. If changes are related or more complex, it would be good to start a discussion on how to approach this, before making a change to ensure it can be merged.

### LICENSE ###
Alien Swarm SDK Copyright(c) Valve Corp.
Valve did not explicitly specify a licence for Alien Swarm SDK but it is most likely similar to SOURCE 1 SDK LICENSE https://github.com/ValveSoftware/source-sdk-2013/blob/master/LICENSE
If you would like to use the SDK for a commercial purpose, please contact Valve at
sourceengine@valvesoftware.com.

### CONTRIBUTING ###
Thanks for your interest in the Alien Swarm: Reactive Drop project.  When you make a
contribution to the project (e.g. create an Issue or submit a Pull Request)
(a "Contribution"), Reactive Drop Team wants to be able to use your Contribution to improve
the game.

As a condition of providing a Contribution, you agree that:

* You irrevocably grant anyone the right to use your work under the following license: Creative Commons CC0 Waiver (release all rights, like public domain: [legal code](https://creativecommons.org/publicdomain/zero/1.0/))
* You warrant and represent that the Contribution is your original creation,
that you have the authority to grant this license to anyone, and that this
license does not require the permission of any third party.  Otherwise, you
provide your Contribution "as is" without warranties.

Should you wish to submit a suggestion or work that is not your original
creation, you may submit it to Reactive Drop Team separate from any Contribution,
explicitly identifying it as sourced from a third party, stating the details
of its origin, and informing Reactive Drop Team of any license or other restriction of
which you are personally aware.


Reactive Drop Team is happy to accept pull requests and issues in the reactivedrop_public_src
repository in these cases:

 * Changes that fix bugs in the SDK deployment process itself. The repository
   should build out of the box, and anything that prevents that is a pull
   request we want.
 * High priority bugs in Alien Swarm: Reactive Drop, that can be fixed in
   client.dll or server.dll.
 * New weapons, NPCs, features.

If you are going to make a pull request, please keep them as granular as
possible. Pull requests with 3-4 unrelated changes in them aren't going to
be accepted.
