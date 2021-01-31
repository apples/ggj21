local vdom = require('vdom')

return function(props)
    local score = "Press F1 to start game once lobby is filled"

    return vdom.create_element('widget', { width = '100%', height = '100%' },
        vdom.create_element(
            'label',
            {
                halign='center',
                valign='top',
                top=0,
                height = 48,
                text = score,
                color = '#888'
            }
        )
    )
end
