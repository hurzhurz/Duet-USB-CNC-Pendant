while global.pendant_continuous_ttl>0
	if global.pendant_continuous_axis=="X"
		G91 G0 X{global.pendant_continuous_step} F{global.pendant_continuous_feed}
	elif global.pendant_continuous_axis=="Y"
		G91 G0 Y{global.pendant_continuous_step} F{global.pendant_continuous_feed}
	elif global.pendant_continuous_axis=="Z"
		G91 G0 Z{global.pendant_continuous_step} F{global.pendant_continuous_feed}
	set global.pendant_continuous_ttl = global.pendant_continuous_ttl-1