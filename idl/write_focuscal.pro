pro write_focuscal, fcal

pname = strcompress(getenv('VISAO_ROOT') + '/calib/visao/focus/focus.cal')

forprint, textout=pname, fcal, /NOCOMMENT, /silent


end
