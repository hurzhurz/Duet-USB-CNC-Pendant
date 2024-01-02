if exists(param.A) && exists(param.F) && exists(param.D)
    set global.pendant_continuous_axis = {param.A}
    set global.pendant_continuous_feed = {param.F}
    set global.pendant_continuous_forward = {param.D==1}
    set global.pendant_continuous_step = {(global.pendant_continuous_feed/600)/20}
    if param.D!=1
        set global.pendant_continuous_step = {-global.pendant_continuous_step}
    set global.pendant_continuous_ttl = 150
else
    echo "not enough parameters"