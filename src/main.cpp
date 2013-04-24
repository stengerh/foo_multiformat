// foo_multiformat.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

DECLARE_COMPONENT_VERSION(
"Multi-formatting Tech Demo",
"1.0 alpha 2013-04-24",
"This component is a tech demo. As such it may not be as stable as a production quality release.\n"
"The component demonstrates a title formatting technique which allows using arbitrary functions on "
"the individual values of multi-value fields. "
"For example, the expression \"$left(%<artist>%,1)|%<artist>%\" should work like most users expect."
"Moreover, it will produce the same result as \"$left($put(A,%<artist>%),1)|$get(A)\".\n"
"\n"
"Copyright (C) 2008-2013 Holger Stenger"
)

VALIDATE_COMPONENT_FILENAME("foo_multiformat.dll")
