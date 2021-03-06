#include <pthread.h>
#include <iostream>

using namespace std;
 
class Value {
public:
    Value() : _value(0) { }
 
    void update(int value) {
        _value = value;
    }
 
    int get() const {
        return _value;
    }
 
private:
    int _value;
};

pthread_t producer;
pthread_t consumer;
pthread_t interrupter;
pthread_mutex_t mutex;
pthread_cond_t cond;

pthread_mutex_t cons_mutex;
pthread_cond_t cons_cond;


int condition = 0;
int consumer_starts=0;
bool continue_updates = true;

 
void* producer_routine(void* arg) {
  // Wait for consumer to start
    while (consumer_starts == 0)
    {
        pthread_cond_wait( &cons_cond, &cons_mutex );
    }
    int n;   // number of values
    int x;   // values
    cout << "Enter the number of values: ";
    cin >> n;
    cout << "Enter the values: ";
    Value *value = (Value*)arg;
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    while( n > 0 ) 
    { 
        pthread_mutex_lock( &mutex ); 
        while( condition == 1 ) 
            pthread_cond_wait( &cond, &mutex ); 
        cin >> x;
        value->update(x);
        n--;
        condition = 1; 
        pthread_cond_signal( &cond ); 
        pthread_mutex_unlock( &mutex ); 
    } 
    continue_updates = false;
    
    
}
 
void* consumer_routine(void* arg) {
  // notify about start
  // allocate value for result
  // for every update issued by producer, read the value and add to sum
  // return pointer to result
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_mutex_lock( &cons_mutex );
    consumer_starts = 1;
    pthread_cond_signal( &cons_cond );
    pthread_mutex_unlock( &cons_mutex );
    int *sum = new int();
    Value *value = (Value*)arg;
    *sum = 0;
    while(continue_updates) 
    { 
        
        pthread_mutex_lock( &mutex ); 
        while( condition == 0 ) 
            pthread_cond_wait( &cond, &mutex ); 
        *sum += value->get();
        condition = 0; 
        pthread_cond_signal( &cond ); 
        pthread_mutex_unlock( &mutex ); 
    }   
    return sum;
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
    while (consumer_starts == 0)
    {
        pthread_cond_wait( &cons_cond, &cons_mutex );
    }
  // interrupt consumer while producer is running   
    while(continue_updates)
    {
        pthread_cancel(consumer);
    }                                       
}
 
int run_threads() {
  // start 3 threads and wait until they're done
    Value *value = new Value();
    int *sum;
    

    pthread_create(&producer,NULL,&producer_routine,value);
    pthread_create(&consumer,NULL,&consumer_routine,value);
    pthread_create(&interrupter,NULL,&consumer_interruptor_routine,NULL);

  // return sum of update values seen by consumer
    pthread_join(producer,NULL);
    pthread_join(consumer,(void**)&sum); 
    pthread_join(interrupter,NULL);
    
    int ans = *sum;
    delete sum;
    return ans;
}
 
int main() {
    cout << run_threads() << endl;
    return 0;
}
