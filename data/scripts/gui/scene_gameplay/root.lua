local vdom = require('vdom')

-- score_black: number
-- score_white: number
return function(props)
    local score = props.score_black .. ' | ' .. props.score_white

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
