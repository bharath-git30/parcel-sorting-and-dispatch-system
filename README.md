# Parcel Sorting and Dispatch System

A console-based C project demonstrating a queue, stack, binary search tree,
hash table, and circular linked list through a parcel-sorting simulation.

The project files are in **Parcel Sorting and Dispatch System**.

## Run on Windows

Download the repository using **Code > Download ZIP**, extract it, open the
project folder, and double-click **Run.bat**.

## Use the application

1. Add parcels by selecting a destination, priority, and size.
2. Choose **Process one tick** to sort and attempt dispatch.
3. Inspect arrival queues, city queues, returns, or search by parcel ID.
4. Choose **Exit** when finished.

Adding or inspecting parcels does not advance the simulation. Priority and
size are informational. Failed dispatches return parcels for later processing.
Records last for the current session; the app does not restore a previous run.

## Files

- `Source Code/src`: C implementation files.
- `Source Code/include`: C header files.
- `Source Code/Build.bat`: rebuild with GCC or a supported MSVC environment.
- `parcel_system.exe`: compiled Windows application.
- `Run.bat`: application launcher.
- `Abstract.pdf`: project abstract.

If no `config.txt` is supplied, the app uses its built-in settings: arrival
capacity 30, three sorts, two reprocessings and two dispatch attempts per tick,
five ticks per terminal, 10% simulated failure probability, and random seed 42.

The included version is intended for small demonstrations. Input validation
is limited, and large custom batch limits can corrupt the displayed summaries.
