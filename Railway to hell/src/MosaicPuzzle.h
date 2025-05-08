#ifndef __MOSAIC_PUZZLE_H__
#define __MOSAIC_PUZZLE_H__

#include "MosaicPiece.h"
#include <vector>

// This is a manager class
class MosaicPuzzle
{
public:
    MosaicPuzzle();
    virtual ~MosaicPuzzle();

    // Initialize the puzzle
    bool Initialize();

    // Update logic
    bool Update(float dt);

    // Clean up resources
    bool CleanUp();

    // Add a piece to the puzzle
    void AddPiece(MosaicPiece* piece);

    // Rotate a specific piece by ID
    void RotatePiece(int pieceId);

    // Check if the puzzle is solved
    bool IsSolved() const;

    // Get a piece by its ID
    MosaicPiece* GetPieceById(int id);

    // Event when the puzzle is solved
    void OnPuzzleSolved();

public:
    // Sound effect IDs
    int solveFxId;
    int rotateFxId;

private:
    // List of pieces that make up this puzzle
    std::vector<MosaicPiece*> pieces;

    // Flag to track if the puzzle has been solved
    bool solved;
};

#endif // __MOSAIC_PUZZLE_H__