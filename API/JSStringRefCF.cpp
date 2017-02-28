#include "APICast.h"
#include "JSStringRef.h"

#include <kjs/JSLock.h>
#include <kjs/JSType.h>
#include <kjs/internal.h>
#include <kjs/operations.h>
#include <kjs/ustring.h>
#include <kjs/value.h>

using namespace KJS;

JSStringRef JSStringCreateWithCFString(CFStringRef string)
{
    JSLock lock;
    CFIndex length = CFStringGetLength(string);

    // Optimized path for when CFString backing store is a UTF16 buffer
    if (const UniChar* buffer = CFStringGetCharactersPtr(string)) {
        UString::Rep* rep = UString(reinterpret_cast<const UChar*>(buffer), length).rep()->ref();
        return toRef(rep);
    }

    UniChar* buffer = static_cast<UniChar*>(fastMalloc(sizeof(UniChar) * length));
    CFStringGetCharacters(string, CFRangeMake(0, length), buffer);
    UString::Rep* rep = UString(reinterpret_cast<UChar*>(buffer), length, false).rep()->ref();
    return toRef(rep);
}

CFStringRef JSStringCopyCFString(CFAllocatorRef alloc, JSStringRef string)
{
    UString::Rep* rep = toJS(string);
    return CFStringCreateWithCharacters(alloc, reinterpret_cast<const JSChar*>(rep->data()), rep->size());
}
