#pragma once
// grouped together .h's inferred from official MSVC documentation
#include "xapilibi/debugapi.h"
#include "xapilibi/errhandlingapi.h"
#include "xapilibi/fileapi.h"
#include "xapilibi/handleapi.h"
#include "xapilibi/processenv.h"
#include "xapilibi/processthreadsapi.h"
#include "xapilibi/stringapiset.h"
#include "xapilibi/synchapi.h"
#include "xapilibi/sysinfoapi.h"
#include "xapilibi/timezoneapi.h"
#include "xapilibi/winbase.h"
// standalone loose .h files inferred from vibes
#include "xapilibi/bgdownload.h"
#include "xapilibi/getlocale.h"
#include "xapilibi/getlang.h"
#include "xapilibi/nuiapithunk.h"
#include "xapilibi/physical.h"
#include "xapilibi/scrnsavr.h"
#include "xapilibi/uiapithunk.h"
#include "xapilibi/xcontent.h"
#include "xapilibi/xcontentcrosstitle.h"
#include "xapilibi/xgetoverlappedextendederror.h"
#include "xapilibi/xgetoverlappedresult.h"
#include "xapilibi/xinpapi.h"
#include "xapilibi/xlaunch.h"
#include "xapilibi/xoverlap.h"
#include "xapilibi/xthread.h"
#include "xapilibi/xuser.h"
#include "xapilibi/xusergetsignininfo.h"

// this is where you'd include every last .h file that you need that comes from XAPILIB
// that way, you don't have to worry about which XAPILIB header to include for your
// particular use case, as you can just include this one
