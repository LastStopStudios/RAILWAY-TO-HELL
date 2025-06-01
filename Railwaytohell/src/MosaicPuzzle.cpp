#include "MosaicPuzzle.h"
#include "Engine.h"
#include "Audio.h"
#include "Log.h"
#include "Scene.h"
#include "UI.h"

MosaicPuzzle::MosaicPuzzle() : solved(false), solveFxId(0), rotateFxId(0)
{
}

MosaicPuzzle::~MosaicPuzzle()
{
}

bool MosaicPuzzle::Initialize() {
    return true;
}

bool MosaicPuzzle::Update(float dt)
{
    if (Engine::GetInstance().scene->GetCurrentState() != SceneState::GAMEPLAY)
    {
        return true;
    }

    // Check if the puzzle is solved
    if (!solved && IsSolved()) {
        OnPuzzleSolved();
    }

    return true;
}

bool MosaicPuzzle::CleanUp()
{
    // We don't delete the pieces here as they are managed by the scene
    pieces.clear();
    return true;
}

void MosaicPuzzle::RotatePiece(int pieceId)
{
    MosaicPiece* piece = GetPieceById(pieceId);
    if (piece != nullptr) {
        piece->Rotate();

        // Play rotation sound effect if valid
        if (rotateFxId > 0) {
            Engine::GetInstance().audio.get()->PlayFx(rotateFxId);
        }

        // Check if the puzzle is solved after rotation
        if (IsSolved() && !solved) {
            OnPuzzleSolved();
            Engine::GetInstance().ui->item = 5;
            Engine::GetInstance().ui->PopeadaTime = true;
        }
    }
}

bool MosaicPuzzle::IsSolved() const
{
    // If there are no pieces, the puzzle can't be solved
    if (pieces.empty()) {
        return false;
    }

    // Check if all pieces are in their correct rotation
    for (auto piece : pieces) {
        if (!piece->IsCorrectRotation()) {
            return false;
        }
    }
    return true;
}

MosaicPiece* MosaicPuzzle::GetPieceById(int id)
{
    for (auto piece : pieces) {
        if (piece != nullptr) {
            if (piece->GetPieceId() == id ||
                piece->name == "mosaic_piece_" + std::to_string(id)) {
                return piece;
            }
        }
    }
    return nullptr;
}

void MosaicPuzzle::AddPiece(MosaicPiece* piece)
{
    if (piece != nullptr) {
        pieces.push_back(piece);
        LOG("MosaicPuzzle: Added piece ID %d", piece->GetPieceId());
    }
}

void MosaicPuzzle::OnPuzzleSolved()
{
    Engine::GetInstance().scene->SetOpenDoors();
    solved = true;
    LOG("Mosaic puzzle solved!");

    // Play solve sound effect if valid
    if (solveFxId > 0) {
        Engine::GetInstance().audio.get()->PlayFx(solveFxId);
    }
}