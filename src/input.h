#pragma once

#include <glm/glm.hpp>

class Input {
public:
    static constexpr size_t k_buttons_count = 3;
    static double           s_double_click_time; // mouse double click time in seconds.

    enum class ButtonID : uint8_t
    {
        left   = 0,
        right  = 1,
        middle = 2
    };

    enum class Mods : uint32_t
    {
        shift     = 0x0001,
        ctrl      = 0x0002,
        alt       = 0x0004,
        supper    = 0x0008,
        ctrl_shif = ctrl | shift
    };

    enum class Action : uint8_t
    {
        none    = 0,
        press   = 1,
        drag    = 2,
        release = 3
    };

    enum class DragConstrain : uint8_t
    {
        unconstrained = 0,
        horizontal    = 1,
        vertical      = 2
    };

    struct Button
    {
        Action action = Action::none;
        // a bitwise combination of Mods keys
        uint32_t modifier = 0;
        // screen coordinates that button was initially pressed, zero if no action
        glm::vec2 pressed_position = {0.f, 0.f};
        // drag vector from pressed position, still valid if pressed is released
        glm::vec2     drag            = {0.f, 0.f};
        DragConstrain drag_constraint = DragConstrain::unconstrained;
        bool          is_doubleclick  = false;
        // the last time the button was pressed
        double trigger_time = 0;

        void reset()
        {
            action           = Action::none;
            modifier         = 0;
            pressed_position = glm::vec2(0.f, 0.f);
            drag             = glm::vec2(0.f, 0.f);
            drag_constraint  = DragConstrain::unconstrained;
            is_doubleclick   = false;
            trigger_time     = 0;
        }
    };

    void setButtonEvent(uint32_t  button_index,
                        Action    action,
                        uint32_t  mods,
                        glm::vec2 position,
                        double    time);

    void setPointerPosition(glm::vec2 position);

    void reset(size_t mb_index)
    {
        assert(mb_index < k_buttons_count);
        if (mb_index >= k_buttons_count)
            return;
        mbs_[mb_index].reset();
    }

    void reset()
    {
        for (size_t index = 0; index < k_buttons_count; index++)
            mbs_[index].reset();
    }

    // call at the end of frame before polling new events
    void consume()
    {
        // any button release event that we didn't explicity use must be consumed, and the button
        // state must be reset
        for (size_t index = 0; index < k_buttons_count; index++)
        {
            if (mbs_[index].action == Action::release)
            {
                mbs_[index].reset();
            }
        }
    }

private:
    Button    mbs_[k_buttons_count];
    glm::vec2 cursor_pos_ {0.f, 0.f};
};

void Input::setButtonEvent(uint32_t  button_index,
                           Action    action,
                           uint32_t  mods,
                           glm::vec2 position,
                           double    time)
{
    if (button_index >= k_buttons_count)
        return;

    Button& button = mbs_[button_index];

    if (action == Action::press)
    {
        // if the button is pressed and was previously in a neutral state, it could be a click or a
        // double-click
        if (button.action == Action::none)
        {
            // a double click event is recorded if the new click follows a prevous click of the same
            // button within some time interval
            button.is_doubleclick = (time - button.trigger_time) < s_double_click_time;
            button.trigger_time   = time;

            // record position of the pointer during the press event, we're going to use it to
            // determine drag operations
            button.pressed_position = position;

            // the state of modifiers are recorded when buttons are initially pressed. The state is
            // retained for the duration of click/drag. We don't update this state until the next
            // button press event
            button.modifier = mods;
        }

        button.action = Action::press;
    }
    else if (action == Action::release)
    {
        // when button is released, record any drag distance
        if (button.action != Action::none)
        {
            button.drag = position - button.pressed_position;
        }

        button.action = Action::release;
    }
}

void Input::setPointerPosition(glm::vec2 position)
{
    cursor_pos_ = position;

    // update each button action. Each button holding a press event becomes a drag event
    for (size_t mb_index = 0; mb_index < k_buttons_count; mb_index++)
    {
        Button& button = mbs_[mb_index];
        if (button.action == Action::none)
            continue;

        button.drag        = position - button.pressed_position;
        const bool is_move = button.drag != glm::vec2(0, 0);

        if (is_move && button.action == Action::press)
        {
            button.action = Action::drag;

            if (button.modifier & static_cast<uint32_t>(Mods::shift))
            {
                button.drag_constraint = fabs(button.drag.x) > fabs(button.drag.y) ?
                                             DragConstrain::horizontal :
                                             DragConstrain::vertical;
            }
            else
            {
                button.drag_constraint = DragConstrain::unconstrained;
            }
        }
    }
}
