#pragma once

/**
 * @brief base class for states
 */
class State
{
  public:
    /**
     * @brief enter state
     */
    virtual void enter()
    {
    }

    /**
     * @brief exit state
     */
    virtual void exit()
    {
    }

    /**
     * @brief Event handler called once per tick in this state
     *
     * @return a new state
     */
    virtual State *tick()
    {
        return this;
    }
};
