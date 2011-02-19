//
//	lang.h
//

#ifndef __LANG_H__
#define __LANG_H__

#include "wx/string.h"

#include "config.h"

#ifdef ENABLE_NLS
#include <map>
#include <vector>

typedef enum {
	Catalan, ChineseSimp, Danish, Dutch, English, Esperanto, Finnish,
	French, German, Greek, Italian, Norwegian, Polish, Portuguese,
	Romanian, Spanish, Swedish, SwedishChef
} Language;


// A phrase (in English), and all its translations
// TODO: Make this a base class, and derive classes for the different types
class PhraseBlock
{
public:
	typedef enum { regular, concat, literal, replace } type_t;

private:
	wxString english;
	type_t type;

	// for 'regular' type
	std::map<Language, wxString> translations;

	// for 'concat' type
	std::vector<wxString> concats;

	// for 'replace' type
	wxString base, substr, substr_new;

public:
	PhraseBlock (wxString eng, type_t type = regular);
	~PhraseBlock ();

	type_t get_type () const { return type; }
	void set_type (type_t type);
	void set_param (int id, wxString param);

	void add (Language lang, wxString translation);
	bool has_translation (Language lang) const;
	const wxString lookup (Language lang) const;

	int verify_phrase () const;
};

typedef struct {
	char *english, *other;
} raw_trans;

class Translator
{
private:
	std::map<const wxString, PhraseBlock *> phrases;

	Language default_lang;

	void split_lang_spec (const char *spec, wxString &LL, wxString &CC) const;

public:
	static int warn_untranslated;

	Translator (int debug = 0);
	~Translator ();

	void init ();
	static wxString get_language_name (Language lang);
	static Language get_language_from_name (const wxString lang);

	void verify_phrases () const;

	bool guess_language (bool ignore_cc = false);
	void set_language (Language lang);
	Language get_language () const;

	void add_phrase (PhraseBlock *phr);

	const wxString lookup (Language lang, const char *s) const;
	const wxString lookup (Language lang, const wxString s) const;
	const wxString lookup (const wxString s) const;

	// avoid calling this, unless necessary
	PhraseBlock *lookup_phr (const wxString s) const;
};


extern Translator *translator;	// defined in main.cc

#define _(str)		(translator->lookup (wxT(str)))

#else	// ! ENABLE_NLS

#define _(str)		(wxT(str))

#endif


#endif	// __LANG_H__
