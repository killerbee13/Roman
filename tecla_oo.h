/*
	* I make no copyright claim to the contents of this file, believing it to
	* be trivial work obvious to anyone with knowledge of the field. Therefore,
	* it is in the public domain.
*/

#ifndef TECLA_OOP_H_INCLUDED
#define TECLA_OOP_H_INCLUDED

#include <libtecla.h>

//Simple RAII wrapper around libtecla
class Tecla_gl {
public:
	Tecla_gl(size_t linelen, size_t histlen, const char * filename, int histlines=1000)
		: _gl(new_GetLine(linelen, histlen)), _filename(nullptr), _histlines(histlines)
		{load_history();}
	~Tecla_gl()
		{save_history(); del_GetLine(_gl);}
	
	char * getline(const char * prompt, const char * start_line, int start_pos)
		{return gl_get_line(_gl, prompt, start_line, start_pos);}
	
	GlReturnStatus status() 
		{return gl_return_status(_gl);}
	const char * error_message(char * buff=nullptr, size_t size=0)
		{return gl_error_message(_gl, buff, size);}
	
	int save_history()
		{return gl_save_history(_gl, _filename, "#", _histlines);}
	int load_history()
		{return gl_load_history(_gl, _filename, "#");}
	
	//If _gl is not null
	explicit operator bool()
		{return !!_gl;}
	
	GetLine * get_gl() {return _gl;}
private:
	GetLine *_gl;
	const char * _filename;
	int _histlines;
};

#endif //TECLA_OOP_H_INCLUDED