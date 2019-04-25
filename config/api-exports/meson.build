if host_machine.system() == 'windows'
    config.set('API_EXPORTED', '__declspec(dllexport)')
    config.set('LOCAL_FN', '')
    config.set('API_EXPORTED_RELOCATABLE', '__declspec(dllexport)')
    config.set('DEPRECATED', '__attribute__ ((deprecated))')
    config.set('HOTSPOT', '__attribute__ ((hot))')
else
    config.set('API_EXPORTED', '__attribute__ ((visibility ("protected")))')
    config.set('LOCAL_FN', '__attribute__ ((visibility ("hidden")))')
    config.set('API_EXPORTED_RELOCATABLE', '__attribute__ ((visibility ("default")))')
    config.set('DEPRECATED', '__attribute__ ((deprecated))')
    config.set('HOTSPOT', '__attribute__ ((hot))')
endif


