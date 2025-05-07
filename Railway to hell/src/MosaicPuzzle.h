#pragma once

#include <vector>
#include "Entity.h"
#include "MosaicPiece.h"

class MosaicPuzzle : public Entity
{
public:
    MosaicPuzzle();
    virtual ~MosaicPuzzle();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    // Rotate a specific piece by its ID
    void RotatePiece(int pieceId);

    // Check if the puzzle is solved
    bool IsSolved() const;

    // Get a piece by its ID
    MosaicPiece* GetPieceById(int id);

    // Add a new piece to the puzzle
    void AddPiece(MosaicPiece* piece);

    // Event triggered when puzzle is solved
    void OnPuzzleSolved();

    void SetParameters(pugi::xml_node parameters) {
        this->parameters = parameters;
    }

private:
    std::vector<MosaicPiece*> pieces;
    bool solved;

    // Sound effects
    unsigned int solveFxId;
    unsigned int rotateFxId;

    pugi::xml_node parameters;
};