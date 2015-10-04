[![Build Status](https://travis-ci.org/alexshafranov/derplanner.png?branch=master)](https://travis-ci.org/alexshafranov/derplanner)
----------
derplanner
==========
* derplanner is a C++ game AI decision making library.  
* derplanner is based on Hierarchical-Task-Network Planning.  
* derplanner is a compiler, it turns the custom domain description language into C++ code.  

*Here's a sneak peek at domain syntax:*
```
domain turret {
  fact location(vec3, vec3)
  fact target(id32, vec3)
  
  prim fire!(id32)
  
  const Fov = cos(pi() / 4.0)
  
  task attack_visible() {
    each :sorted(1.0 - Dot)
      location(Pos, Dir) & target(Id, Tgt) &
      (Dot = dot(norm(Tgt - Pos), Dir)) & (Dot >= Fov) -> [ fire!(Id) ]
  }
}
```

## Building Library
Derplanner uses premake5 to generate visual studio project files / makefiles.  
Pre-built premake5 executable is shipped as part of the project.  

* On Windows run ```premake5 vs2015``` (vs2010, vs2012 and vs2013 should also work)
* On Linux run ```./premake5 gmake```

There're no external dependencies, so alternatively, you can just include derplanner code in your own build system.

## License

derplanner is licensed under [zlib license](./LICENSE.txt)
