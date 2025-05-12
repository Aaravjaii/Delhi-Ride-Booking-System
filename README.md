# Delhi Ride Booking System

A command-line based ride-booking system written in C++ that simulates Uber-like functionality for Delhi city, including driver and user account management, location-based routing, and driver rating features.

## Features

- User Registration and Account Management
- Driver Registration and Availability Management
- Location Graph with Shortest Path Calculation (Dijkstra’s Algorithm)
- Real-time Haversine Distance Calculation between Points
- Wallet and Cash-based Payment System
- Driver Rating System
- Persistent Data using CSV Files

## Directory Structure

📁 project_root/
├── delhi_location_edges.csv # Graph edges (from,to,distance)
├── driver_ratings.csv # Stores driver ratings (phone,rating,count)
├── registered_drivers.csv # List of all registered drivers
├── ride_history.csv # Optional ride logs (future use)
├── user_accounts.csv # Stores user details and wallet info
├── test2.cpp # Main source file (code provided)
└── README.md # This file
