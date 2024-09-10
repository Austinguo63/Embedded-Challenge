Final Project Fall 2023: Need for Speed
Group Number: 39
Auther Name: Hongyu Guo (Hg2860), Zhenyuan Dong(zd2362)
Youtube link: https://youtu.be/Oi5I4e3ZiLI


1.Running Process

(1) We use a tick to sample from the gyroscope per 0.5s, and we use a flag to tag it.
(2) We use the lcd function to creat wonderful pages and can switch it by tapping on the screen.
(3) Start Page: When tapping the “Start” button we can enter the Real Time Data Page.
(4) Real Time Data Page: The page will displays a real-time data status. We can see the instant velocity every 0.5s, the distance we walked and the time we spent. 
(5) Distance Graph: We can see the distance we walked on the graph.
(6) Fianl Total Data Page: After 20s and we can tap the screen to enter the Fianl Total Data Page. We can see the total time, distance and average velocity on it.


2.Core function 

(1) To calculate the instant velocity(m/s), we use the Formula v = rw. We just let r be the Height_Term which correlates with the height of the man who is using the device. And the w is the angular velocities sampled from the one dimension of gyroscope. 
(2) To calculate the final walking distance(m), our group utilized the instantaneous velocity of each sampling interval (0.5s) multiplied by the time interval as the walking distance for this sampling interval, and then added up all the interval distances to get the total distance.
(3) The average velocity is Average speed is the final distance traveled in 20s divided by 20s.

