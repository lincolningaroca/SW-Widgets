#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QStyle>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>

#include "../SWFluentUI/swribbon.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QMainWindow window;
  window.setWindowTitle("SW_Widgets Demo-ribbon");

  //ribbon test
  // 1. Crear la instancia del Ribbon
  SWFluentUI::SWRibbon *ribbon = new SWFluentUI::SWRibbon(&window);

  // 2. Insertarlo en tu diseño de Qt Creator
  // Si tienes un layout en el centralWidget, agrégalo al principio:
  // ui->verticalLayoutMain->insertWidget(0, ribbon);
  // O si quieres que sea el "menuBar" (aunque es un QWidget):
  window.setMenuWidget(ribbon);

  // --- CONFIGURACIÓN DE CONTROLES ---

  // Iconos estándar para el ejemplo
  QIcon iconNew = qApp->style()->standardIcon(QStyle::SP_FileIcon);
  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  QIcon iconEdit = qApp->style()->standardIcon(QStyle::SP_FileDialogContentsView);

  // Pestaña "Inicio" (Home)
  SWFluentUI::SWRibbonPage *homePage = ribbon->addPage("Inicio");

  // Grupo: Portapapeles (Botones Grandes)
  SWFluentUI::SWRibbonGroup *groupClipboard = homePage->addGroup("Portapapeles");
  groupClipboard->addLargeAction(new QAction(iconNew, "Pegar", &window));
  groupClipboard->addSeparator();

  // Botones pequeños (Se apilarán en 3 filas automáticamente)
  groupClipboard->addSmallAction(new QAction(iconEdit, "Cortar", &window));
  groupClipboard->addSmallAction(new QAction(iconSave, "Copiar", &window));
  groupClipboard->addSmallAction(new QAction(iconEdit, "Pintar", &window));

  // Grupo: Edición (Controles de Entrada)
  SWFluentUI::SWRibbonGroup *groupEdit = homePage->addGroup("Edición");

  // Podemos insertar cualquier QWidget directamente
  QLineEdit *searchField = new QLineEdit();
  searchField->setPlaceholderText("Buscar...");
  searchField->setFixedWidth(120);
  groupEdit->addWidget(searchField);

  QComboBox *fontCombo = new QComboBox();
  fontCombo->addItems({"Segoe UI", "Arial", "Consolas", "Roboto"});
  groupEdit->addWidget(fontCombo);

  // Grupo: Configuración (Widgets de estado)
  SWFluentUI::SWRibbonGroup *groupSettings = homePage->addGroup("Opciones");

  QCheckBox *checkGrid = new QCheckBox("Ver Cuadrícula");
  QSpinBox *zoomSpin = new QSpinBox();
  zoomSpin->setRange(10, 500);
  zoomSpin->setSuffix("%");
  zoomSpin->setValue(100);

  groupSettings->addWidget(checkGrid);
  groupSettings->addWidget(zoomSpin);

  // --- PESTAÑAS CONTEXTUALES (Peculiares de tu proyecto) ---
  // Aparecen con un color distintivo (por ejemplo, al seleccionar una imagen)
  SWFluentUI::SWRibbonPage *formatPage = ribbon->addContextPage("Herramientas de Imagen", QColor(255, 170, 0)); // Naranja

  SWFluentUI::SWRibbonGroup *imgEffects = formatPage->addGroup("Efectos");
  imgEffects->addLargeAction(new QAction("Brillo", &window));
  imgEffects->addLargeAction(new QAction("Contraste", &window));

  // Mostrar la pestaña contextual (esto lo harías dinámicamente)
  ribbon->setContextVisible("Herramientas de Imagen", true);

  // Acciones de acceso rápido (Quick Access Toolbar)
  ribbon->addQuickAction(new QAction(iconSave, "Guardar", &window));
  ribbon->addQuickAction(new QAction(qApp->style()->standardIcon(QStyle::SP_ArrowBack), "Deshacer", &window));

  ribbon->setCompactMode(true);

  window.resize(300, 200);
  window.show();

  return app.exec();
}