local vdom = require('vdom')

return function(props)
    return vdom.create_element('widget', { width = '100%', height = '100%' })
end
