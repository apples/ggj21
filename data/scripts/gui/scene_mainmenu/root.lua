local vdom = require('vdom')
local textbox = require('gui.controls.textbox')

-- start_game: () => ()
-- set_join_popup: (boolean) => ()
local function mainmenu(props)
    local open_join = vdom.useCallback(function () props.set_join_popup(true) end, {props.set_join_popup})

    return vdom.create_element('widget', { width = '100%', height = '100%' },
        vdom.create_element(
            'label',
            {
                halign = 'center',
                bottom = 720/2 - 20,
                color = '#fff',
                text = '[NEW GAME]',
                height = 24,
                on_click = props.start_game,
            }
        ),
        vdom.create_element(
            'label',
            {
                halign = 'center',
                bottom = 720/2 - 60,
                color = '#fff',
                text = '[JOIN GAME]',
                height = 24,
                on_click = open_join,
            }
        )
    )
end

-- join_game: (string) => ()
-- set_join_popup: (boolean) => ()
local function joinpopup(props)
    local close_join = vdom.useCallback(function () props.set_join_popup(false) end, {props.set_join_popup})

    local addr, set_addr = vdom.useState('')
    
    local join = vdom.useCallback(function () props.join_game(addr) end, {props.join_game, addr})

    return vdom.create_element('widget', { width = '100%', height = '100%' },
        vdom.create_element(
            textbox,
            {
                halign = 'center',
                bottom = 720/2 - 20,
                value = addr,
                on_change = set_addr,
                height = 24,
                width = 300,
            }
        ),
        addr == ''
            and vdom.create_element(
                'label',
                {
                    halign = 'center',
                    bottom = 720/2 - 60,
                    color = '#888',
                    text = '[JOIN]',
                    height = 24,
                }
            )
            or vdom.create_element(
                'label',
                {
                    halign = 'center',
                    bottom = 720/2 - 60,
                    color = '#fff',
                    text = '[JOIN]',
                    height = 24,
                    on_click = join,
                }
            ),
        vdom.create_element(
            'label',
            {
                halign = 'center',
                bottom = 720/2 - 120,
                color = '#fff',
                text = '[BACK]',
                height = 24,
                on_click = close_join,
            }
        )
    )
end

-- start_game: () => ()
-- join_game: (string) => ()
return function(props)
    local join_popup, set_join_popup = vdom.useState(false)

    return vdom.create_element('widget', { width = '100%', height = '100%' },
        vdom.create_element(
            'label',
            {
                halign = 'center',
                bottom = 720/2 + 80,
                color = '#fff',
                text = 'GGJ21',
                height = 24,
            }
        ),
        join_popup
            and vdom.create_element(
                joinpopup,
                {
                    join_game = props.join_game,
                    set_join_popup = set_join_popup,
                }
            )
            or vdom.create_element(
                mainmenu,
                {
                    start_game = props.start_game,
                    set_join_popup = set_join_popup,
                }
            )
    )
end
