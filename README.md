derplanner
==========
[![Build Status](https://travis-ci.org/alexshafranov/derplanner.png?branch=master)](https://travis-ci.org/alexshafranov/derplanner)

**derplanner** is a:

* C++ game AI decision making library.  
* Hierarchical-Task-Network Planner.  
* Compiler, turning the custom domain description language into C++ code.  

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

## Building
**derplanner** has no external dependencies, so it can be included into your own build system.  

Alternatively, you can use premake5 executable shipped with the project to generate project files / makefiles.

* On Windows run ```premake5 vs2015```. Any Visual Studio version, starting from **Visual C++ 2008** should work as well.
* On Linux run ```./premake5 gmake```. Linux build is regularly tested on **gcc 4.6.3** and **clang 3.4**.

## License
derplanner is licensed under [zlib license](./LICENSE.txt)
