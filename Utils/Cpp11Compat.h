/*
	Defines backward compatibility with C++11 standard.
*/

#ifndef __StegoFS__Cpp11Compat__
#define __StegoFS__Cpp11Compat__

// all Visual Studios up to VS2010
#if _MSC_VER <= 1600

#define		KEYWORD_FINAL
#define		FOREACH(_T_, _V_, _E_)	for (vector<_T_>::iterator (_E_) = (_V_).begin(); (_E_) != (_V_).end(); (_E_)++)
#define		FOREACH_ELM(_E_)	(*_E_)

#else // _MSC_VER > 1600 or gcc or something else

#define		KEYWORD_FINAL final
#define		FOREACH(_T_, _V_, _E_)	for (const _T_& (_E_) : (_V_))
#define		FOREACH_ELM(_E_)	(_E_)

#endif // _MSC_VER <= 1600

#endif // __StegoFS__Cpp11Compat__