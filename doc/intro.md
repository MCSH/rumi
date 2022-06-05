# Features List

* function signatures

```rumi
function_name := (LIST_OF_ARGS) -> OUT_TYPE ;

printInt := (i : int) -> unit;
```

* function definition

```rumi
main := () -> int {
  return 0;
}
```

* while loop

```rumi
while (cond){
  statements
}

while (i){
  printInt(i);
  i = i - 1;
}
```

* if statements

```rumi
if (cond) {
  statements
} /* optionally */ else {
  statements
}

if(a){
  printf("a is true\n");
}

if(a){
  printf("a is true\n");
} else {
  printf("a is false\n");
}
```

* variable definition

```rumi
var_name : type = value ;
var_name := value ; // type is inferred
var_name :type; // value is not-initialzied
```

* type casting

```rumi
a : s64 = 2;
c : u16;

c = a -> u16 ;
```

* Pointers

```rumi
*type

// pointer to anything:
* any // void*

p : * int ;
i : int = 2;
*p = i; // copies the value of i to p
p = &i ; // point to i
```

* Structs

```rumi
name : struct {
  fields
}

Person : struct{
  first_name : string;
  last_name  : string;
}


s : Person ;
s.first_name = "Sajjad" ;
s.last_name  = "Heydari";

printf("%s %s\n",
          s.first_name,
          s.last_name);
```

* sizeof
```rumi
sizeof type

sizeof Person

sizeof *Person
```

* Methods

```rumi
struct_name . method_name := function_definition

Person.print := () -> unit {
  printf("<Person: %s %s>", self.first_name, self.last_name);
}

s.print();
```

* Interfaces

```rumi
interface_name : interface{
  method signatures
}

Printable : interface{
  print := () -> unit ;
}

p : Printable ;
p = s -> Printable ;

// TODO: we should be able to do p = s without any casting;
```

* Pointers of structs and interfaces

```rumi
ip : * Printable ;
ip = &p;

ip.print(); // no need to use -> like in C
(*ip).print(); // equivalent to the previous line
```

* Pointer arithmatics

```rumi
array : *int = malloc(3 * sizeof int) -> * int;

*(array + 0) = 1;
*(array + 1) = 2;
*(array + 2) = 4;

printf("[%d, %d, %d]\n", 
       *array, *(array+1), *(array+2));


x := 2;

array[0] = 1;
array[1] = 2;
array[x] = 4;
```

* Importing other files

```rumi
import file_name
```

* compile directive

```rumi
@compile
func_name := () -> int {
  return 0;
}
```

If you return 0, it means it's okay for the compiler to move on, if you return 1, it means the compiler should produce a warning, if you return any other value, the compiler will panic.

The first argument is optionally a pointer to the Compiler struct.

TODO: Take a look at test 7, test 8 and test 9

* Enums

```rumi
name : enum {
  values;
}

E: enum {
  v1 = 1,
  v2,
  v3 = 4,
}

e: E; // int
e = E.v1;
```

* Operator overloading

```rumi

Vector2d: struct{
  x: int;
  y: int;
}

// arguments could be a direct instance or a pointer to the types.
// return type is treated as is.
Vector2d + Vector2d := (lhs: Vector2d, rhs: *Vector2d) -> Vector2d{
  ans : Vector2d;
  ans.x = lhs.x + rhs.x;
  ans.y = lhs.y + rhs.y;
  // proof that the second one is a pointer:
  rhs.x = 0;
  rhs.y = 0;
  return ans;
}
```

Regardless of whether lhs or rhs are poitners or direct values, the compiler should be able to handle all of the following:

```rumi
pos      : Vector2d;
movement : Vector2d;

pos = pos  + movement  ; // value + value
pos = pos  + &movement ; // value + pointer
pos = &pos + movement  ; // pointer + value
pos = &pos + &movement ; // pointer + pointer
```

* Cosntructor

```rumi
new Vector2d := () -> unit{
  self.x = 0;
  self.y = 0;
}
```

* using statement

```rumi
Vector2d + Vector2d := (lhs: *Vector2d, rhs: *Vector2d) -> Vector2d{
  ans : Vector2d;
  using ans;

  x = lhs.x + rhs.x; // ans.x = ...
  y = lhs.y + rhs.y; // ans.y = ...

  return ans;
}
```
