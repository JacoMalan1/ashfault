#include <ashfault/editor/editor.h>

namespace ashfault::editor {
Editor::Editor(std::shared_ptr<Engine> engine, std::shared_ptr<Window> window)
    : Application(engine, window) {}

Editor::~Editor() {
}

void Editor::run() {
}
} // namespace ashfault::editor
