Import("env")



# Ensure the correct libraries and include paths are used
env.Append(
    CPPPATH=["/usr/local/include/libelf/", "/usr/local/opt/ncurses/include"],
    LIBPATH=["/usr/local/opt/ncurses/lib", "/usr/local/lib/"],
    LIBS=["elf", "ncurses", "simavr"]
)



env.Append(LINKFLAGS=["-L/usr/local/lib", "-lsimavr", "-lsimavrparts", "-lelf"])