#pragma once

#include <QWidget>
#include <QTabBar>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QToolButton>
#include <QAction>
#include <QToolBar>
#include <QMap>
#include <QColor>
#include <QMenu>
#include <QLabel>
#include <QPropertyAnimation>
#include <QEvent>
#include <QWheelEvent>

namespace SWFluentUI {

class SWRibbonPage;
class SWRibbonGroup;

// ── SWRibbonTabBar ────────────────────────────────────────────────────────────
class SWRibbonTabBar : public QTabBar
{
  Q_OBJECT
public:
  explicit SWRibbonTabBar(QWidget *parent = nullptr);
  void setTabColor(int index, const QColor &color);

protected:
  void paintEvent(QPaintEvent *event) override;
  void changeEvent(QEvent *event)     override;

private:
  QMap<int, QColor> m_colors;
};

// ── SWRibbon ──────────────────────────────────────────────────────────────────
class SWRibbon : public QWidget
{
  Q_OBJECT

  // Propiedad animable dedicada — sin conflicto con el layout manager
  Q_PROPERTY(int ribbonHeight READ ribbonHeight WRITE setRibbonHeight)
  Q_PROPERTY(bool minimized   READ isMinimized   WRITE setMinimized   NOTIFY minimizedChanged)
  Q_PROPERTY(bool compactMode READ isCompactMode WRITE setCompactMode NOTIFY compactModeChanged)

public:
  explicit SWRibbon(QWidget *parent = nullptr);

  SWRibbonPage* addPage(const QString &title);
  SWRibbonPage* addContextPage(const QString &title, const QColor &color);
  void          setContextVisible(const QString &title, bool visible);
  void          addQuickAction(QAction *action);

  bool isMinimized()   const { return m_minimized;   }
  bool isCompactMode() const { return m_compactMode; }

  // Alturas configurables
  void setNormalHeight(int h);
  void setCompactHeight(int h);
  void setMinimizedHeight(int h);

public slots:
  void setMinimized(bool minimized);
  void setCompactMode(bool compact);
  void toggleMinimized();

signals:
  void minimizedChanged(bool minimized);
  void compactModeChanged(bool compact);

protected:
  void resizeEvent(QResizeEvent *event)          override;
  void wheelEvent(QWheelEvent *event)            override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void changeEvent(QEvent *event)                override;

private slots:
  void onTabChanged(int index);

private:
  int  ribbonHeight() const { return m_ribbonHeight; }
  void setRibbonHeight(int h);

  void updateHeightWithAnimation();
  void recalcGroupCollapse();

  struct ContextTab { int index; QColor color; };
  QMap<QString, ContextTab> m_contextTabs;

  QWidget*            m_topContainer { nullptr };
  QToolBar*           m_quickAccess  { nullptr };
  SWRibbonTabBar*     m_tabBar       { nullptr };
  QStackedWidget*     m_stack        { nullptr };
  QToolButton*        m_toggleBtn    { nullptr };
  QPropertyAnimation* m_animation    { nullptr };

  bool m_minimized   { false };
  bool m_compactMode { false };

  int m_normalHeight    { 150 };
  int m_compactHeight   { 130 };
  int m_minimizedHeight { 35  };
  int m_ribbonHeight    { 150 };
};

// ── SWRibbonPage ──────────────────────────────────────────────────────────────
class SWRibbonPage : public QWidget
{
  Q_OBJECT
public:
  explicit SWRibbonPage(QWidget *parent = nullptr);
  SWRibbonGroup*        addGroup(const QString &title);
  QList<SWRibbonGroup*> groups() const { return m_groups; }

private:
  QHBoxLayout*          m_layout { nullptr };
  QList<SWRibbonGroup*> m_groups;
};

// ── SWRibbonGroup ─────────────────────────────────────────────────────────────
class SWRibbonGroup : public QFrame
{
  Q_OBJECT
public:
  explicit SWRibbonGroup(const QString &title, QWidget *parent = nullptr);

  void addLargeAction(QAction *action);
  void addSmallAction(QAction *action);
  void addWidget(QWidget *w);
  void addSeparator();

  void setCollapsed(bool collapsed);
  void setCompactMode(bool compact);

  // Filas máximas para acciones pequeñas (default: 3)
  void setSmallActionRows(int rows);
  int  smallActionRows() const { return m_smallActionRows; }

  QSize sizeHint() const override;

protected:
  void changeEvent(QEvent *event) override;

private:
  void rebuildOverflowMenu();
  void updateTitleStyle();

  QVBoxLayout* m_mainLayout    { nullptr };
  QHBoxLayout* m_contentLayout { nullptr };
  QGridLayout* m_smallLayout   { nullptr };
  QWidget*     m_contentWidget { nullptr };
  QLabel*      m_titleLabel    { nullptr };
  QToolButton* m_overflowButton{ nullptr };
  QMenu*       m_overflowMenu  { nullptr };

  // Lista explícita — evita findChildren() frágil en colapso
  QList<QToolButton*> m_actionButtons;

  int  m_smallCount      { 0 };
  int  m_smallActionRows { 3 };
  bool m_collapsed       { false };
  bool m_compactMode     { false };
};

} // namespace SWFluentUI