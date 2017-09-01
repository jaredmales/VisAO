function get_focuscal

pname = strcompress(getenv('VISAO_ROOT') + '/calib/visao/focus/focus.cal')

readcol, pname, fcal

return, fcal

end
