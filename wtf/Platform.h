// -*- mode: c++; c-basic-offset: 4 -*-
/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KXMLCORE_PLATFORM_H
#define KXMLCORE_PLATFORM_H

// PLATFORM handles OS, operating environment, graphics API, and CPU
#define PLATFORM(KX_FEATURE) (defined( KXMLCORE_PLATFORM_##KX_FEATURE ) && KXMLCORE_PLATFORM_##KX_FEATURE)
#define COMPILER(KX_FEATURE) (defined( KXMLCORE_COMPILER_##KX_FEATURE ) && KXMLCORE_COMPILER_##KX_FEATURE)
#define CPU(KX_FEATURE) (defined( KXMLCORE_CPU_##KX_FEATURE ) && KXMLCORE_CPU_##KX_FEATURE)
#define HAVE(KX_FEATURE) (defined( HAVE_##KX_FEATURE ) && HAVE_##KX_FEATURE)
#define USE(KX_FEATURE) (defined( KXMLCORE_USE_##KX_FEATURE ) && KXMLCORE_USE_##KX_FEATURE)

// Operating systems - low-level dependencies

// PLATFORM(DARWIN)
// Operating system level dependencies for Mac OS X / Darwin that should
// be used regardless of operating environment
#ifdef __APPLE__
#define KXMLCORE_PLATFORM_DARWIN 1
#endif

// PLATFORM(WIN_OS)
// Operating system level dependencies for Windows that should be used
// regardless of operating environment
#if defined(WIN32) || defined(_WIN32)
#define KXMLCORE_PLATFORM_WIN_OS 1
#endif

// PLATFORM(ANDROID)
// Operating system level dependencies for Android that should be used
// regardless of operating environment
#if defined(ANDROID)     \
 || defined(__ANDROID__)
#define KXMLCORE_PLATFORM_ANDROID 1
#endif

// PLATFORM(UNIX)
// Operating system level dependencies for Unix-like systems that
// should be used regardless of operating environment
// (includes PLATFORM(DARWIN))
#if   defined(__APPLE__)   \
   || defined(unix)        \
   || defined(__unix)      \
   || defined(__unix__)    \
   || defined (__NetBSD__) \
   || defined(_AIX)        \
   || defined(__linux__)   \
   || defined(ANDROID)     \
   || defined(__ANDROID__)
#define KXMLCORE_PLATFORM_UNIX 1
#endif

// Operating environments

// I made the BUILDING_KDE__ macro up for the KDE build system to define

// PLATFORM(KDE)
// PLATFORM(MAC)
// PLATFORM(WIN)
#if defined(BUILDING_KDE__)
#define KXMLCORE_PLATFORM_KDE 1
#elif PLATFORM(DARWIN)
#define KXMLCORE_PLATFORM_MAC 1
#elif PLATFORM(WIN_OS)
#define KXMLCORE_PLATFORM_WIN 1
#endif
#if defined(BUILDING_GDK__)
#define KXMLCORE_PLATFORM_GDK 1
#endif


/* === CPU() - the target CPU architecture ==== */

/* CPU(ALPHA) - DEC Alpha */
#if defined(__alpha__)
#define KXMLCORE_CPU_ALPHA 1
#endif

/* CPU(IA64) - Itanium / IA-64 */
#if defined(__ia64__)
#define KXMLCORE_CPU_IA64 1
/* 32-bit mode on Itanium */
#if !defined(__LP64__)
#define KXMLCORE_CPU_IA64_32 1
#endif
#endif

/* CPU(PPC) - PowerPC 32-bit */
#if    defined(__ppc__)     \
    || defined(__PPC__)     \
    || defined(__powerpc__) \
    || defined(__powerpc)   \
    || defined(__POWERPC__) \
    || defined(_M_PPC)      \
    || defined(__PPC)
#define KXMLCORE_CPU_PPC 1
#define KXMLCORE_CPU_BIG_ENDIAN 1
#endif

/* CPU(PPC64) - PowerPC 64-bit */
#if    defined(__ppc64__) \
    || defined(__PPC64__)
#define KXMLCORE_CPU_PPC64 1
#define KXMLCORE_CPU_BIG_ENDIAN 1
#endif

/* CPU(SH4) - SuperH SH-4 */
#if defined(__SH4__)
#define KXMLCORE_CPU_SH4 1
#endif

/* CPU(SPARC32) - SPARC 32-bit */
#if defined(__sparc) && !defined(__arch64__) || defined(__sparcv8)
#define KXMLCORE_CPU_SPARC32 1
#define KXMLCORE_CPU_BIG_ENDIAN 1
#endif

/* CPU(SPARC64) - SPARC 64-bit */
#if defined(__sparc__) && defined(__arch64__) || defined (__sparcv9)
#define KXMLCORE_CPU_SPARC64 1
#define KXMLCORE_CPU_BIG_ENDIAN 1
#endif

/* CPU(SPARC) - any SPARC, true for CPU(SPARC32) and CPU(SPARC64) */
#if CPU(SPARC32) || CPU(SPARC64)
#define KXMLCORE_CPU_SPARC 1
#endif

/* CPU(S390X) - S390 64-bit */
#if defined(__s390x__)
#define KXMLCORE_CPU_S390X 1
#define KXMLCORE_CPU_BIG_ENDIAN 1
#endif

/* CPU(S390) - S390 32-bit */
#if defined(__s390__)
#define KXMLCORE_CPU_S390 1
#define KXMLCORE_CPU_BIG_ENDIAN 1
#endif

/* CPU(X86) - i386 / x86 32-bit */
#if    defined(__i386__) \
    || defined(i386)     \
    || defined(_M_IX86)  \
    || defined(_X86_)    \
    || defined(__THW_INTEL)
#define KXMLCORE_CPU_X86 1
#endif

/* CPU(X86_64) - AMD64 / Intel64 / x86_64 64-bit */
#if    defined(__x86_64__) \
    || defined(_M_X64)
#define KXMLCORE_CPU_X86_64 1
#endif

/* CPU(ARM) - ARM, any version*/
#if    defined(arm) \
    || defined(__arm__) \
    || defined(ARM) \
    || defined(_ARM_)
#define KXMLCORE_CPU_ARM 1

#if defined(__ARMEB__) || (COMPILER(RVCT) && defined(__BIG_ENDIAN))
#define KXMLCORE_CPU_BIG_ENDIAN 1

#elif  !defined(__ARM_EABI__) \
    && !defined(__EABI__) \
    && !defined(__VFP_FP__) \
    && !defined(_WIN32_WCE) \
    && !defined(ANDROID)
#define KXMLCORE_CPU_MIDDLE_ENDIAN 1

#endif

#if defined(__ARM_NEON__) && !defined(KXMLCORE_CPU_ARM_NEON)
#define KXMLCORE_CPU_ARM_NEON 1
#endif

/* CPU(ARM64) - ARM, 64 */
#if    defined(__aarch64__)
#define KXMLCORE_CPU_ARM64 1
#endif

#endif /* ARM */

#if !defined(KXMLCORE_USE_JSVALUE64) && !defined(KXMLCORE_USE_JSVALUE32_64)
#if (CPU(X86_64) && (PLATFORM(UNIX) || PLATFORM(WINDOWS))) \
    || (CPU(IA64) && !CPU(IA64_32)) \
    || CPU(ALPHA) \
    || CPU(SPARC64) \
    || CPU(S390X) \
    || CPU(PPC64) \
    || CPU(ARM64)
#define KXMLCORE_USE_JSVALUE64 1
#else
#define KXMLCORE_USE_JSVALUE32_64 1
#endif
#endif /* !defined(KXMLCORE_USE_JSVALUE64) && !defined(KXMLCORE_USE_JSVALUE32_64) */


// Compiler

// COMPILER(MSVC)
#if defined(_MSC_VER)
#define KXMLCORE_COMPILER_MSVC 1
#endif

// COMPILER(GCC)
#if defined(__GNUC__)
#define KXMLCORE_COMPILER_GCC 1
#endif

// COMPILER(BORLAND)
// not really fully supported - is this relevant any more?
#if defined(__BORLANDC__)
#define KXMLCORE_COMPILER_BORLAND 1
#endif

// COMPILER(CYGWIN)
// not really fully supported - is this relevant any more?
#if defined(__CYGWIN__)
#define KXMLCORE_COMPILER_CYGWIN 1
#endif

// multiple threads only supported on Mac for now
#if PLATFORM(MAC)
#define KXMLCORE_USE_MULTIPLE_THREADS 1
// don't build with Core Foundation support
#undef KXMLCORE_USE_APPLE_CF
#endif

// for Unicode, KDE uses Qt, everything else uses ICU
#if PLATFORM(KDE)
#define KXMLCORE_USE_QT4_UNICODE 1
#else
//#define KXMLCORE_USE_ICU_UNICODE 1
#define KXMLCORE_USE_LIBC_UNICODE 1
#endif

#endif // KXMLCORE_PLATFORM_H
