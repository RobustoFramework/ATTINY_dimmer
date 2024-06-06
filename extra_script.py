Import("env")
env.Append(LINKFLAGS=["-L/usr/local/lib", "-lsimavr", "-lsimavrparts", "-lelf", "-lncurses"])
