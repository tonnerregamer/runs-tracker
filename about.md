# Runs Tracker

A lightweight utility to automatically record and analyze attempt segments in both Normal and Practice modes.

## Key Features
- **Start Pos Accuracy**: Calculates absolute progress (`playerX / levelLength`) rather than native relative percentage, ensuring Start Pos attempts are correctly tracked.
- **Pathfinding Algorithm**: Sorts attempts and calculates the minimum combination of runs required to cover the level from 0% to 100%.
- **Editor Merging**: Link an online level with a local editor copy. Statistics from both will automatically merge for seamless practice sessions.
- **Clean UI**: Simple, paginated local level browser (6 items per page, sorted newest first) to prevent game lag.

## How to Use
- Click the green **Runs** button on the left panel (online levels) or below the back button (editor).
- To link an online level to an editor copy, open the stats on the online level and click **Link**, then select your local level.
- Click **Unlink** in the stats window at any time to split them.