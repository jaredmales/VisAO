function get_focuspreset, pname

pname = strcompress(getenv('VISAO_ROOT') + '/calib/visao/focus/presets/' + pname + '.preset')

readcol, pname, fpreset, /silent

return, fpreset[0]

end
