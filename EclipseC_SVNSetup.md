# Instructions for setting up eclipse/svn environment #

# Following steps run under Ubuntu 12.04: #

  1. Install eclipse, under "Ubuntu Software Center". **(skip this step if eclipse already installed)**
  1. Within Eclispe:
    1. Help/Install New Software/Add:
> > > Name: SVN ..
> > > Location: http://subclipse.tigris.org/update_1.8.x
    1. select both "Subclipse" and SVNKit" and follow instructions to install, then allow to reboot eclipse
    1. After eclipse restarts:
> > > File/New Project/Checkout Projects from SVN
    1. Create New Repository, and enter:
> > > https://all-eyes.googlecode.com/svn/trunk/ **(for ae-daemon/ae-proxy)**
> > > > or

> > > https://all-eyes.googlecode.com/svn/trunk/src/aemgr/aeManager **(for aeManager)**


> After hitting next, the source tree should display.

> (NOTE: User should also install CDT to allow for easy editing of "c" programs.)