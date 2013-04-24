This component is a tech demo. As such it may not be as stable as a production
quality release.
The component demonstrates a title formatting technique which allows using
arbitrary functions on the individual values of multi-value fields. For
example, the expression

  $left(%<artist>%,1)|%<artist>%

should work like most users expect. Moreover, it will produce the same result
as 

  $left($put(A,%<artist>%),1)|$get(A)
