#include "MosaicPuzzle.h"
#include "Engine.h"
#include "Audio.h"
#include "Log.h"
#include "Scene.h"

MosaicPuzzle::MosaicPuzzle() : Entity(EntityType::UNKNOWN), solved(false)
{
}

MosaicPuzzle::~MosaicPuzzle() {}

bool MosaicPuzzle::Awake() {
    return true;
}

bool MosaicPuzzle::Start() {
    // Load sound effects
    solveFxId = Engine::GetInstance().audio.get()->LoadFx(parameters.attribute("solve_fx").as_string("Assets/Audio/Fx/puzzle_solved.wav"));
    rotateFxId = Engine::GetInstance().audio.get()->LoadFx(parameters.attribute("rotate_fx").as_string("Assets/Audio/Fx/piece_rotate.wav"));

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
        Engine::GetInstance().audio.get()->PlayFx(rotateFxId);

        // Check if the puzzle is solved after rotation
        if (IsSolved() && !solved) {
            OnPuzzleSolved();
        }
    }
}

bool MosaicPuzzle::IsSolved() const
{
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
        // Need to use getter method as pieceId is private
        if (piece != nullptr) {
            // Assuming we have a GetPieceId() method or we can access by other means
            if (piece->name == "mosaic_piece_" + std::to_string(id)) {
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
    }
}

void MosaicPuzzle::OnPuzzleSolved()
{
    solved = true;
    LOG("Mosaic puzzle solved!");
    Engine::GetInstance().audio.get()->PlayFx(solveFxId);

    // Here you can add logic to trigger events when the puzzle is solved
    // For example, open a door, reveal a hidden item, etc.
}