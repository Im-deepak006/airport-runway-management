
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PLANES 10
#define FILE_NAME "runway_queue.txt"
#define LOG_FILE "action_log.txt" // Remove this redundant definition inside the function



typedef struct {
    int id;
    int fuelLevel;
    int arrivalTime;
    int emergency;  // 1 for emergency, 0 otherwise
    int canceled;   // 1 if the plane's landing is canceled
} Plane;

typedef struct {
    Plane* planes;
    int count;
    int capacity;
} PriorityQueue;

// Function prototypes
void initQueue(PriorityQueue* queue);
void enqueue(PriorityQueue* queue, Plane plane);
Plane dequeue(PriorityQueue* queue);
Plane peek(const PriorityQueue* queue);
int isRunwayFree(const PriorityQueue* queue);
void heapifyUp(PriorityQueue* queue, int index);
void heapifyDown(PriorityQueue* queue, int index);
void swap(Plane* a, Plane* b);
void displayQueue(const PriorityQueue* queue);
void resizeQueue(PriorityQueue* queue);
void saveQueueToFile(const PriorityQueue* queue);
void loadQueueFromFile(PriorityQueue* queue);
void cancelLanding(PriorityQueue* queue, int planeId);
void adjustQueueBasedOnTime(PriorityQueue* queue);
void logAction(const char* action, const Plane* plane);


// Function to read valid integer input
int getValidIntegerInput(int min, int max) {
    int value;
    while (1) {
        printf("Enter a Fuel Level between %d and %d: ", min, max);

        if (scanf("%d", &value) != 1 || value < min || value > max) {
            printf("Invalid input. Please enter a value between %d and %d.\n", min, max);
            while (getchar() != '\n');
        } else {
            return value;
        }
    }
}

// Initialize the priority queue
void initQueue(PriorityQueue* queue) {
    queue->count = 0;
    queue->capacity = MAX_PLANES;
    queue->planes = (Plane*)malloc(sizeof(Plane) * queue->capacity);
}

// Resize the queue when it's full
void resizeQueue(PriorityQueue* queue) {
    int oldCapacity = queue->capacity;
    queue->capacity *= 2;
    queue->planes = (Plane*)realloc(queue->planes, sizeof(Plane) * queue->capacity);
    printf("Queue resized from %d to %d planes.\n", oldCapacity, queue->capacity);
}

// Insert a plane into the queue and re-heapify
void enqueue(PriorityQueue* queue, Plane plane) {
    if (queue->count >= queue->capacity) {
        resizeQueue(queue);
    }
    queue->planes[queue->count] = plane;
    heapifyUp(queue, queue->count);
    queue->count++;
    printf("Plane %d added to the queue.\n", plane.id);
 logAction("Enqueue", &plane);



}

// Remove the highest-priority plane from the queue and re-heapify
Plane dequeue(PriorityQueue* queue) {
    if (queue->count <= 0) {
        printf("\nRunway is empty. No planes are waiting to land.\n");
        Plane emptyPlane = {0, 0, 0, 0, 0};
        return emptyPlane;
    }

    Plane top = queue->planes[0];
    queue->planes[0] = queue->planes[--queue->count];
    heapifyDown(queue, 0);

    printf("\nNotification: Plane %d cleared for landing.\n", top.id);
    printf("Details:\n");
    printf("  Fuel Level    : %d\n", top.fuelLevel);
    printf("  Arrival Time  : %d\n", top.arrivalTime);
    printf("  Emergency     : %s\n", top.emergency ? "Yes" : "No");
    printf("  Landing Canceled: %s\n", top.canceled ? "Yes" : "No");

    if (queue->count == 0) {
        printf("\nAll planes have been dequeued. The queue is now empty.\n");
    }
   logAction("Dequeue", &top); // Assuming 'top' is the dequeued plane

    return top;
}

// Peek at the highest-priority plane without removing it
Plane peek(const PriorityQueue* queue) {
    if (queue->count <= 0) {
        printf("\nRunway is empty.\n");
        Plane emptyPlane = {0, 0, 0, 0, 0};
        return emptyPlane;
    }
    return queue->planes[0];
}

// Check if the runway is free (queue is empty)
int isRunwayFree(const PriorityQueue* queue) {
    return queue->count == 0;
}

// Maintain the max-heap property by moving a plane up
void heapifyUp(PriorityQueue* queue, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;

        // Prioritize by emergency first
        if (queue->planes[index].emergency > queue->planes[parent].emergency) {
            swap(&queue->planes[index], &queue->planes[parent]);
            index = parent;
        }
        // If emergency is the same, prioritize by fuel level
        else if (queue->planes[index].emergency == queue->planes[parent].emergency &&
                 queue->planes[index].fuelLevel < queue->planes[parent].fuelLevel) {
            swap(&queue->planes[index], &queue->planes[parent]);
            index = parent;
        }
        // If both emergency and fuel level are the same, prioritize by arrival time
        else if (queue->planes[index].emergency == queue->planes[parent].emergency &&
                 queue->planes[index].fuelLevel == queue->planes[parent].fuelLevel &&
                 queue->planes[index].arrivalTime < queue->planes[parent].arrivalTime) {
            swap(&queue->planes[index], &queue->planes[parent]);
            index = parent;
        } else {
            break; // No need to move up if the plane is correctly positioned
        }
    }
}


// Maintain the max-heap property by moving a plane down
void heapifyDown(PriorityQueue* queue, int index) {
    while (index * 2 + 1 < queue->count) { // While there's at least one child
        int left = index * 2 + 1;
        int right = index * 2 + 2;
        int largest = index;

        // Compare with left child
        if (queue->planes[left].emergency > queue->planes[largest].emergency ||
            (queue->planes[left].emergency == queue->planes[largest].emergency &&
             queue->planes[left].fuelLevel < queue->planes[largest].fuelLevel) ||
            (queue->planes[left].emergency == queue->planes[largest].emergency &&
             queue->planes[left].fuelLevel == queue->planes[largest].fuelLevel &&
             queue->planes[left].arrivalTime < queue->planes[largest].arrivalTime)) {
            largest = left;
        }

        // Compare with right child
        if (right < queue->count &&
            (queue->planes[right].emergency > queue->planes[largest].emergency ||
             (queue->planes[right].emergency == queue->planes[largest].emergency &&
              queue->planes[right].fuelLevel < queue->planes[largest].fuelLevel) ||
             (queue->planes[right].emergency == queue->planes[largest].emergency &&
              queue->planes[right].fuelLevel == queue->planes[largest].fuelLevel &&
              queue->planes[right].arrivalTime < queue->planes[largest].arrivalTime))) {
            largest = right;
        }

        // If the largest is not the current index, swap and continue
        if (largest != index) {
            swap(&queue->planes[index], &queue->planes[largest]);
            index = largest;
        } else {
            break; // Heap property is satisfied
        }
    }
}

// Swap two planes
void swap(Plane* a, Plane* b) {
    Plane temp = *a;
    *a = *b;
    *b = temp;
}

// Display the queue
void displayQueue(const PriorityQueue* queue) {
    if (queue->count == 0) {
        printf("\nNo planes in the queue.\n");
        return;
    }

    printf("\nPlanes in the queue (prioritized by emergency, fuel level, and arrival time):\n");
    printf("ID\tFuel\tArrival\tEmergency\tCanceled\n");
    for (int i = 0; i < queue->count; i++) {
        printf("%d\t%d\t%d\t%d\t\t%d\n", queue->planes[i].id, queue->planes[i].fuelLevel,
               queue->planes[i].arrivalTime, queue->planes[i].emergency, queue->planes[i].canceled);
    }
}

// Save the queue to a text file
void saveQueueToFile(const PriorityQueue* queue) {
    FILE* file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        printf("Error saving queue to file.\n");
        return;
    }

    // Write the number of planes in the queue
    fprintf(file, "Total planes in queue: %d\n", queue->count);
    fprintf(file, "-------------------------------------------------\n");

    // Write header for better readability
    fprintf(file, "%-5s %-10s %-15s %-10s %-10s\n", "ID", "Fuel Level", "Arrival Time", "Emergency", "Canceled");
    fprintf(file, "-------------------------------------------------\n");

    // Write the plane details in a readable format
    for (int i = 0; i < queue->count; i++) {
        fprintf(file, "%-5d %-10d %-15d %-10d %-10d\n",
                queue->planes[i].id,
                queue->planes[i].fuelLevel,
                queue->planes[i].arrivalTime,
                queue->planes[i].emergency,
                queue->planes[i].canceled);
    }

    fclose(file);
    printf("Queue saved to file.\n");
}

// Load the queue from a text file
void loadQueueFromFile(PriorityQueue* queue) {
    FILE* file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        printf("Error loading queue from file.\n");
        return;
    }
    fscanf(file, "%d", &queue->count);
    for (int i = 0; i < queue->count; i++) {
        fscanf(file, "%d %d %d %d %d", &queue->planes[i].id, &queue->planes[i].fuelLevel,
               &queue->planes[i].arrivalTime, &queue->planes[i].emergency, &queue->planes[i].canceled);
    }
    fclose(file);
    printf("Queue loaded from file.\n");
}

// Cancel a plane's landing clearance
void cancelLanding(PriorityQueue* queue, int planeId) {
    for (int i = 0; i < queue->count; i++) {
        if (queue->planes[i].id == planeId) {
            queue->planes[i].canceled = 1;
            heapifyDown(queue, i);  // Re-heapify to maintain priority
            printf("Plane %d's landing clearance has been canceled.\n", planeId);
            logAction("Cancel Landing", &queue->planes[i]);
            return;
        }
    }
    printf("Plane with ID %d not found in the queue.\n", planeId);
}

// Re-heapify based on arrival time


// Adjust the queue based on time (you can add custom logic here)
void adjustQueueBasedOnTime(PriorityQueue* queue) {
    // For simplicity, we'll just re-heapify the entire queue
    for (int i = queue->count / 2 - 1; i >= 0; i--) {
        heapifyDown(queue, i);
    }
    printf("Queue adjusted based on time.\n");
}

// Log an action to the log file
#define LOG_FILE "action_log.txt"

void logAction(const char* action, const Plane* plane) {
    FILE* file = fopen(LOG_FILE, "a");
    if (file == NULL) {
        printf("Error writing to log file.\n");
        return;
    }

    // Log the action performed
    fprintf(file, "Action: %s\n", action);

    // If there's a plane associated with the action, log its details
    if (plane != NULL) {
        fprintf(file, "Plane ID       : %d\n", plane->id);
        fprintf(file, "Fuel Level     : %d\n", plane->fuelLevel);
        fprintf(file, "Arrival Time   : %d\n", plane->arrivalTime);
        fprintf(file, "Emergency      : %s\n", plane->emergency ? "Yes" : "No");
        fprintf(file, "Canceled       : %s\n", plane->canceled ? "Yes" : "No");
    }

    // Separate different actions with a line for readability
    fprintf(file, "----------------------------------------\n");

    fclose(file);
    printf("Action logged.\n");
}

int main() {
    PriorityQueue runwayQueue;
    initQueue(&runwayQueue);

    int choice, planeId = 1, time = 0;
    while (1) {
        printf("\n\n--- Airport Runway Management ---\n");
        printf("1. Enqueue Plane\n");
        printf("2. Dequeue Plane for Landing\n");
        printf("3. Peek at Next Plane\n");
        printf("4. Check if Runway is Free\n");
        printf("5. Display Queue\n");
        printf("6. Save Queue to File\n");
        printf("7. Load Queue from File\n");
        printf("8. Cancel Landing Clearance\n");
        printf("9. Adjust Queue Based on Time\n");
        printf("10. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                Plane newPlane;
                newPlane.id = planeId++;
                newPlane.fuelLevel = getValidIntegerInput(0, 100);
                newPlane.arrivalTime = ++time;

                if (newPlane.fuelLevel < 10) {
                    newPlane.emergency = 1;
                    printf("Fuel level is below 10. Automatically considered as an emergency.\n");
                } else {
                    printf("Is it an emergency landing? (1 for Yes, 0 for No): ");
                    scanf("%d", &newPlane.emergency);
                }

                newPlane.canceled = 0;
                enqueue(&runwayQueue, newPlane);
                break;
            }
            case 2: {
                if (!isRunwayFree(&runwayQueue)) {
                    dequeue(&runwayQueue);
                } else {
                    printf("Runway is free. No planes waiting to land.\n");
                }
                break;
            }
            case 3: {
                Plane nextPlane = peek(&runwayQueue);
                if (nextPlane.id != 0) {
                    printf("Next plane to land: ID %d, Fuel %d, Arrival %d, Emergency %d, Canceled %d\n",
                           nextPlane.id, nextPlane.fuelLevel, nextPlane.arrivalTime, nextPlane.emergency, nextPlane.canceled);
                }
                break;
            }
            case 4: {
                if (isRunwayFree(&runwayQueue)) {
                    printf("Runway is free.\n");
                } else {
                    printf("Runway is occupied.\n");
                }
                break;
            }
            case 5:
                displayQueue(&runwayQueue);
                break;
            case 6:
                saveQueueToFile(&runwayQueue);
                break;
            case 7:
                loadQueueFromFile(&runwayQueue);
                break;
            case 8: {
                int planeIdToCancel;
                printf("Enter the ID of the plane to cancel: ");
                scanf("%d", &planeIdToCancel);
                cancelLanding(&runwayQueue, planeIdToCancel);
                break;
            }
            case 9:
                adjustQueueBasedOnTime(&runwayQueue);
                break;
            case 10:
                printf("Exiting Airport Runway Management.\n");
                free(runwayQueue.planes);
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}