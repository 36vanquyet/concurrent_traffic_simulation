#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return!_queue.empty(); });
    T message = std::move(_queue.back());
    _queue.clear();

    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        if (_messages.receive() == TrafficLightPhase::green)
        {
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases?��? should be started in a thread when the public method „simulate?��? is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::random_device random_number;
    std::mt19937 random_generator(random_number());
    std::uniform_int_distribution<std::mt19937::result_type> distribution(4, 6);

    int cycleTime = distribution(random_generator);
    auto startTime = std::chrono::high_resolution_clock::now();
    
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto elapsedTime = std::chrono::high_resolution_clock::now() - startTime;

        if (std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count() > cycleTime)
        {
            _currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;
            _messages.send(std::move(_currentPhase));
            startTime = std::chrono::high_resolution_clock::now(); // Update time when start new cycle
            cycleTime = distribution(random_generator); // Update cycle time
        }
    }
}
