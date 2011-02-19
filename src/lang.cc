//
//	lang.cc
//

#include "wx/string.h"

#include <iostream>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <vector>
#include "lang.h"

#include "phrases.h"


int Translator::warn_untranslated;


static struct lang_info {
	char *code;	// ISO639 code (e.g. "de")
	char *name;	// full name (e.g. "German")
	Language lang;	// enum value (e.g. German)
} languages[] = {
	{ "ca", "Catalan", Catalan },
	{ "zh", "ChineseSimp", ChineseSimp },
	{ "da", "Danish", Danish },
	{ "nl", "Dutch", Dutch },
	{ "en", "English", English },
	{ "eo", "Esperanto", Esperanto },
	{ "fi", "Finnish", Finnish },
	{ "fr", "French", French },
	{ "de", "German", German },
	{ "el", "Greek", Greek },
	{ "it", "Italian", Italian },
	{ "no", "Norwegian", Norwegian },
	{ "pl", "Polish", Polish },
	{ "pt", "Portuguese", Portuguese },
	{ "ro", "Romanian", Romanian },
	{ "es", "Spanish", Spanish },
	{ "sv", "Swedish", Swedish },
	{ "sv_CK", "SwedishChef", SwedishChef } // CK is Cook Islands (geddit?)
};
#define NUM_LANGUAGES	(int) (sizeof (languages) / sizeof (languages[0]))


PhraseBlock::PhraseBlock (wxString eng, type_t type)
		: english (eng), type (type)
{
}

PhraseBlock::~PhraseBlock ()
{
}

void PhraseBlock::set_type (type_t type)
{
	this->type = type;
}

void PhraseBlock::set_param (int id, wxString param)
{
	if (type == concat) {
		concats.push_back (param);
		return;
	} else if (type == replace) {
		if (id == 0)
			base = param;
		else if (id == 1)
			substr = param;
		else if (id == 2)
			substr_new = param;
		else
			std::cerr << "Bad id for replace in set_param().\n";
		return;
	}

	std::cerr << "Bad set_param() call.\n";
}

void PhraseBlock::add (Language lang, wxString translation)
{
	if (type != regular) {
		std::cerr << "Adding a translation to non-regular block!\n";
		return;
	}
	translations[lang] = translation;
}

bool PhraseBlock::has_translation (Language lang) const
{
	if (lang == English)
		return true;
	if (type == concat) {
		PhraseBlock *phr;
		std::vector<wxString>::const_iterator it;
		for (it = concats.begin (); it != concats.end (); ++it) {
			phr = translator->lookup_phr (*it);
			if (!phr->has_translation (lang))
				return false;
		}
		return true;
	}
	if (type == literal)
		return true;
	if (type == replace) {
		PhraseBlock *phr;
		phr = translator->lookup_phr (base);
		if (!phr)
			return false;
		return phr->has_translation (lang);
	}

	// must be a regular phrase
	std::map<Language, wxString>::const_iterator it;
	it = translations.find (lang);
	if (it == translations.end ())
		return false;
	return true;
}

const wxString PhraseBlock::lookup (Language lang) const
{
	std::map<Language, wxString>::const_iterator it;

	if (lang == English)
		return english;
	if (type == concat) {
		// special case: concat
		wxString ret;
		std::vector<wxString>::const_iterator it;
		for (it = concats.begin (); it != concats.end (); ++it)
			ret += translator->lookup (lang, *it);
		return ret;
	} else if (type == literal) {
		// special case: literal
		return english;
	} else if (type == replace) {
		// special case: replace
		wxString str = translator->lookup (lang, base);
		str.Replace (substr.c_str (), substr_new.c_str ());
		return str;
	}

	it = translations.find (lang);
	if (it == translations.end ()) {
		// fallback to English
		if (Translator::warn_untranslated)
			std::cerr << "WARNING: \"" << english.mb_str (wxConvUTF8)
				<< "\" doesn't have a translation for "
				<< Translator::get_language_name (lang).mb_str (wxConvUTF8)
				<< ".\n";
		return english;
	}
	return it->second;
}

int PhraseBlock::verify_phrase () const
{
	static const wxString punct = wxT(".:!");
	int tail_len, warns;
	std::map<Language, wxString>::const_iterator it;

	warns = 0;

	// Test 0: Only attempt verification if this is a regular phrase
	if (type != regular)
		return 0;

	// Test 1: If English phrase ends in a certain punctuation substring,
	//		then all the translations must, too.
	for (tail_len = 1; tail_len < 5; ++tail_len) {
		wxString prop_tail = english.Right (tail_len);
		if (!punct.Contains (prop_tail.Left (1))) {
			--tail_len;
			break;
		}
	}
	if (tail_len > 0) {
		const wxString tail = english.Right (tail_len);

		for (it = translations.begin (); it != translations.end ();
									++it) {
			Language lang = it->first;
			wxString trans = it->second;
			const wxString tail_trans = trans.Right (tail_len);
			if (!tail.IsSameAs (tail_trans)) {
				std::cerr << "WARNING: \""
					<< english.mb_str (wxConvUTF8)
					<< "\"\n\tends with \""
					<< tail.mb_str (wxConvUTF8)
					<< "\", but the translation for "
					<< Translator::get_language_name (lang).mb_str (wxConvUTF8)
					<< " doesn't!\n";
				++warns;
			}
		}
	}

	// Test 2: If English phrase includes "&", so must the translations.
	static const wxString amp = wxT("&");
	if (english.Contains (amp)) {
		for (it = translations.begin (); it != translations.end ();
									++it) {
			Language lang = it->first;
			wxString trans = it->second;
			if (!trans.Contains (amp)) {
				std::cerr << "WARNING: \""
					<< english.mb_str (wxConvUTF8)
					<< "\"\n\thas a hotkey specifier (&),"
					<< " but the translation for "
					<< Translator::get_language_name (lang).mb_str (wxConvUTF8)
					<< " doesn't!\n";
				++warns;
			}
		}
	}

	// TODO
	// Test 3: If English has a substitution sequence (e.g. "%i"), then
	//		the translations must, too. (and in the right order!)



	return warns;
}

PhraseBlock *Translator::lookup_phr (const wxString s) const
{
	std::map<const wxString, PhraseBlock *>::const_iterator it;

	it = phrases.find (s);
	if (it == phrases.end ())
		return 0;		// no translations found for phrase
	return it->second;
}

void Translator::split_lang_spec (const char *spec, wxString &LL, wxString &CC)
									const
{
	wxString in = wxString (spec, wxConvUTF8);
	int pos;

	pos = in.Find ('_');
	if (pos < 0) {
		LL = in;
		CC = wxT("");
		return;
	}
	LL = in.Mid (0, pos);
	CC = in.Mid (pos + 1);
}

Translator::Translator (int debug)
{
	default_lang = English;
	warn_untranslated = debug;
}

Translator::~Translator ()
{
	std::map<const wxString, PhraseBlock *>::const_iterator it;

	for (it = phrases.begin (); it != phrases.end (); ++it)
		delete it->second;
}

const char *yy_lang_data;
void Translator::init ()
{
#if 0
	// debug
	extern int yy_lang_debug;
	yy_lang_debug = 1;
#endif
	extern int yy_lang_parse ();

	yy_lang_data = phrases_raw;
	if (yy_lang_parse ()) {
		// error!
		throw std::runtime_error ("Bad phrase file!");
	}
}

wxString Translator::get_language_name (Language lang)
{
	for (int i = 0; i < NUM_LANGUAGES; ++i)
		if (languages[i].lang == lang)
			return wxString (languages[i].name, wxConvUTF8);
	return wxT("Unknown");
}

Language Translator::get_language_from_name (const wxString lang)
{
	for (int i = 0; i < NUM_LANGUAGES; ++i) {
		if (lang == wxString (languages[i].name, wxConvUTF8)) {
			return languages[i].lang;
		}
	}

	return English;		// TODO: anything better we can do here?
}

void Translator::verify_phrases () const
{
	std::map<const wxString, PhraseBlock *>::const_iterator it;
	int tot_warns = 0, tot_missing = 0, total = 0;

	for (it = phrases.begin (); it != phrases.end (); ++it) {
		PhraseBlock *phr = it->second;
		tot_warns += phr->verify_phrase ();

		// don't check 'concat' phrase blocks, because their parts
		// will be caught later (or earlier)
		if (phr->get_type () == PhraseBlock::concat)
			continue;
		if (!phr->has_translation (default_lang)) {
			tot_missing += 1;
			std::cerr << "\"" << it->first.mb_str (wxConvUTF8) << "\"\n";
		}
		total += 1;
	}

	if (tot_warns > 0) {
		std::cerr << "-------------\n";
		std::cerr << tot_warns << " warning" <<
			(tot_warns > 1 ? "s" : "") << ".\n";
		std::cerr << "-------------\n";
	}
	if (tot_missing > 0) {
		double perc = 100 - (tot_missing * 100.0) / total;
		std::cerr << "-------------\n";
		std::cerr << "\""
			<< Translator::get_language_name (default_lang).mb_str (wxConvUTF8)
			<< "\" is missing " << tot_missing << "/" << total
			<< " translations (" << perc << "% complete).\n";
		std::cerr << "-------------\n";
	}
}

bool Translator::guess_language (bool ignore_cc)
{
	// First, check environment variables
	char *envvars[3] = { "LANGUAGE", "LC_ALL", "LANG" };
	for (int i = 0; i < 3; ++i) {
		char *lang = getenv (envvars[i]);
		if (!lang)
			continue;
		wxString LL, CC;
		if (ignore_cc)
			split_lang_spec (lang, LL, CC);
		else
			LL = wxString (lang, wxConvUTF8);
		//std::cerr << "Looking for '" << LL << "'.\n";
		for (int j = 0; j < NUM_LANGUAGES; ++j) {
			if (LL == wxString (languages[j].code, wxConvUTF8)) {
				// Found it!
				set_language (languages[j].lang);
				return true;
			}
		}
	}

	// Failed!

	if (ignore_cc)
		return false;	// can't do any more

	// Try without country code
	return guess_language (true);
}

void Translator::set_language (Language lang)
{
	default_lang = lang;
}

Language Translator::get_language () const
{
	return default_lang;
}

void Translator::add_phrase (PhraseBlock *phr)
{
	wxString eng = phr->lookup (English);
	phrases[eng] = phr;
}

const wxString Translator::lookup (Language lang, const char *s) const
{
	return lookup (lang, wxString (s, wxConvUTF8));
}

const wxString Translator::lookup (Language lang, const wxString s) const
{
	PhraseBlock *phr;

	if (lang == English)
		return s;
	phr = lookup_phr (s);
	if (!phr) {
		// fallback to English
		if (warn_untranslated)
			std::cerr << "WARNING: \"" << s.mb_str (wxConvUTF8)
					<< "\" doesn't have a PhraseBlock.\n";
		return s;
	}
	return phr->lookup (lang);
}

const wxString Translator::lookup (const wxString s) const
{
	return lookup (default_lang, s);
}
