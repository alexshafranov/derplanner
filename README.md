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

## Quick Intro

### Types
* ```id32```, ```id64``` 32-bit and 64-bit handles, to reference entities.
* ```int8```, ```int32```, ```int64``` signed integers.
* ```float``` 32-bit floating point.
* ```vec3``` 3-component vector type.

### Fact Database
Fact database is a collection of typed tuples, it represents domain knowledge about the world.  

A domain can specify the required database format:  
```fact { location(vec3) target(id32, vec3) }```

### Primitive Task
The resuling plan is a sequence of primitive task instances (i.e. a task  + argument tuple):  
```goto!(A) melee!(T) goto!(B)```  

Primitive tasks can be defined in the following way:  
```prim { goto!(vec3) melee!(id32) }```

### Compound Task
Compound tasks make the planning process hierarchical and recursive.  

A compound task defines one or more *cases*. Each case is a pre-condition and a task list:  
```
task attack(Bot, Enemy) {
  case pos(Bot, Src) & pos(Enemy, Dst) & dist(Src, Dst) < Close_Range
    -> [ approach(Src, Dst), melee!(Bot, Enemy), retreat(Bot) ]
  
  case line_of_sight(Bot, Enemy)
    -> [ select_weapon(Bot), fire!(Enemy) ]
}
```

### Planning
The plan is formulated in a simple *left-to-right* and *top-to-bottom* way:  

* Initially, the plan is a single compound task (the first one defined in a domain).  
* For each compoud task instance in the plan:  
  * For each case, in order of definition:  
    * If the case precondition is satisifed, replace the compound task instance with the task list.  
  * If no satisified cases found, back-track.  

## License
derplanner is licensed under [zlib license](./LICENSE.txt)
