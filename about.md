# Runs Tracker - Progress Analyzer

## Mod Overview
**Runs Tracker** is an analysis and optimization tool designed for Geometry Dash players looking to structure their progression on difficult levels (demons, challenges, etc.).

It automatically records every attempt, or "run" (from your start percentage to your death), and generates an intelligent analysis of your progress. No more guessing your consistency: the mod shows you your precise optimal runs to beat your level.

---

## Main Features

### Automatic and Intelligent Recording
The mod monitors your play sessions in real-time:
- **Precise death tracking**: Every time your icon crashes, the attempt is analyzed and recorded in the background as a percentage segment.
- **Start Pos Support**: Unlike native in-game indicators that sometimes restart at 0% when you place a *Start Pos* object, our mod calculates your player's absolute position by dividing their X coordinate by the total level length. Whether you play in practice mode or with a Start Pos at 90%, your segments will be correctly recorded (for example: 90% - 95% instead of 0% - 5%).
- **Normal & Practice Mode**: Tracking works seamlessly in both game modes to capture all of your training sessions.
- **Win Detection**: When you reach 100% (in either Normal or Practice mode), the mod validates the run and updates the algorithm.

### Progress Analyzer ("Optimal Runs" Algorithm)
Behind the statistics button lies a sorting and pathfinding algorithm:
- **Shortest path calculation**: The algorithm sorts all your recorded runs and calculates the minimum combination of segments needed to cover the level from 0% to 100%.
- **Dynamic display**: If you haven't completed the level at least once yet (in either Normal or Practice mode), the window simply lists your runs compactly. As soon as you cross the finish line, the **"OPTIMAL"** label appears to show you the theoretical minimum number of runs achieved to beat the level.

### Unique Linking System (Online <-> Editor)
A unique feature for challengers:
- **Merged statistics**: You can link an online level with a local copy in your level editor. Once linked, all runs completed in your editor level are added to the online level's runs for a unified, global analysis.
- **Paginated selection interface**: To prevent lag and display bugs if you have hundreds of levels in your editor, the linking menu features a smooth pagination system displaying 6 levels per page, automatically sorted from newest to oldest.
- **Delink Option**: You can unlink two levels at any time directly from the interface without losing your individual run data.

---

## How to Use the Mod?

1. **During your sessions**: Play normally. The mod silently records your data and automatically saves it in two local files located in your Geometry Dash directory: `runs_data.txt` (for percentages) and `links.txt` (for link memory).
2. **Accessing stats (Online & Editor)**:
   - On an online level, click the green **"Runs"** button on the left information menu.
   - On your local editor copies, click the green **"Runs"** button located right below the back button in the edit menu.
3. **Linking/Unlinking a level**:
   - Open the statistics window of an online level.
   - Click the blue **"Lier"** (or Link) button to open your editor levels catalog and select the corresponding copy.
   - If you want to cancel the merge, open the statistics again and click the red **"Delink"** button.

---

## Why Use This Mod?
The original game does not offer a detailed history of your practice attempts. By transforming repetitive failures into actionable mathematical data, this mod allows you to clearly visualize your real progression, pinpoint your weak spots, and approach your play sessions in a much more methodical way.