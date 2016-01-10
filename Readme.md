# autox

This program automatically logs in a specified user in and runs the xinit
program.

On a sysvinit system, the benefit of using this, rather than starting xinit
directly from inittab, is that you get the privileges as defined in the PAM
rules. Also, the program terminates when X is quit. Thus it can be run
repeatedly from inittab and automatically log the user in every time.

*autox* has successfully been used for the same purpose in on an 
[Upstart][upstart] based system, and a [Runit][runit] based system.

[upstart]: http://upstart.ubuntu.com/
[runit]: http://smarden.org/runit/

# Install

Simply `make` and put the binary where you want it. Make sure you install
`autox.pam` as `/etc/pam.d/autox`.

## Upstart Script

```
env USER=username
env DISPLAY=:0.0

description "autox startup script"
author "Start in X session on boot"

emits login-session-start
emits desktop-session-start

start on runlevel [5] #(filesystem and stopped udevtrigger)
stop on runlevel [0136]

respawn

#script
exec /usr/local/bin/autox $USER 2>/tmp/autox.log
#end script
```

## Runit Script

```bash
!/bin/sh
USER=username

exec 2>&1
exec setsid -w agetty -a $USER -n -l /usr/local/bin/autox -o $USER tty7 38400 linux
```

Note: your `.xinitrc` should only `exec` *one* program, and it should remain
in the foreground. If you `exec` anything as a background process then it will
run outside of the `runsvdir` process tree and will not be managed by it.

# License

[MIT License](http://jsumners.mit-license.org/)