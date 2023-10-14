---
--- This script is used to configure to build the project.
--- It is used to check whether the current platform is supported.
---
local configh = require('configh')
local cfgh = configh(os.getenv('CC'))

cfgh:output_status(true)
cfgh:set_feature('_GNU_SOURCE')
for header, funcs in pairs({
    ['signal.h'] = {
        'sigwaitinfo',
        'sigtimedwait',
        'sigisemptyset',
    },
    ['pthread.h'] = {
        'pthread_condattr_setclock',
    },
}) do
    if cfgh:check_header(header) then
        for _, func in ipairs(funcs) do
            cfgh:check_func(header, func)
        end
    end
end
assert(cfgh:flush('src/config.h'))
