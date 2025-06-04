<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Railway to Hell | LastStop Studios</title>
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
  <link href="https://fonts.googleapis.com/css2?family=Press+Start+2P&family=Silkscreen&display=swap" rel="stylesheet">
  <style>
    :root {
      --hell-main: #8a0303;    /* Rojo sangre principal */
      --hell-accent: #ff6b1e;  /* Naranja infernal */
      --hell-dark: #1a0000;    /* Fondo oscuro */
      --hell-light: #f5f749;   /* Amarillo neón */
      --pixel-border: #3a0c0c; /* Borde pixelado */
    }
    
    body {
      background-color: var(--hell-dark);
      background-image: 
        url('assets/pixel-grid.png');
      font-family: 'Silkscreen', cursive;
      color: #e0e0e0;
      line-height: 1.6;
      padding: 0;
      margin: 0;
      image-rendering: pixelated;
      image-rendering: -moz-crisp-edges;
      image-rendering: crisp-edges;
    }
    
    .pixel-container {
      max-width: 1200px;
      margin: 20px auto;
      border: 4px solid var(--hell-main);
      background-color: rgba(26, 0, 0, 0.85);
      box-shadow: 0 0 0 8px var(--hell-accent),
                  0 0 30px rgba(255, 30, 30, 0.6);
      position: relative;
      overflow: hidden;
    }
    
    .pixel-container::before {
      content: "";
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      height: 5px;
      background: linear-gradient(90deg, 
        var(--hell-main), 
        var(--hell-accent), 
        var(--hell-light),
        var(--hell-accent),
        var(--hell-main));
    }
    
    .pixel-header {
      padding: 30px;
      text-align: center;
      background: url('assets/train-pixel-bg.png') center/cover;
      border-bottom: 4px solid var(--hell-main);
      position: relative;
    }
    
    .game-title {
      font-family: 'Press Start 2P', cursive;
      font-size: 3rem;
      color: var(--hell-light);
      text-shadow: 4px 4px 0 var(--hell-main),
                  8px 8px 0 #000;
      margin: 0;
      padding: 20px;
      background-color: rgba(26, 0, 0, 0.7);
      display: inline-block;
      border: 4px solid var(--hell-accent);
    }
    
    .game-tagline {
      font-family: 'Press Start 2P', cursive;
      font-size: 1rem;
      color: var(--hell-accent);
      margin-top: 20px;
      display: inline-block;
      background-color: rgba(26, 0, 0, 0.9);
      padding: 10px 20px;
      border: 3px solid var(--hell-light);
    }
    
    .pixel-section {
      padding: 30px;
      border-bottom: 2px dashed var(--hell-accent);
      position: relative;
    }
    
    .pixel-section:last-child {
      border-bottom: none;
    }
    
    .section-title {
      font-family: 'Press Start 2P', cursive;
      font-size: 1.5rem;
      color: var(--hell-light);
      margin-bottom: 25px;
      padding-bottom: 10px;
      border-bottom: 3px solid var(--hell-accent);
      position: relative;
    }
    
    .section-title::after {
      content: "";
      position: absolute;
      bottom: -6px;
      left: 0;
      width: 100px;
      height: 3px;
      background: var(--hell-light);
    }
    
    .pixel-text {
      font-size: 0.9rem;
      margin-bottom: 20px;
    }
    
    .pixel-list {
      list-style-type: none;
      padding-left: 0;
    }
    
    .pixel-list li {
      margin-bottom: 10px;
      padding-left: 25px;
      position: relative;
      font-size: 0.9rem;
    }
    
    .pixel-list li::before {
      content: ">";
      position: absolute;
      left: 0;
      color: var(--hell-accent);
      font-weight: bold;
    }
    
    .pixel-img {
      image-rendering: pixelated;
      border: 4px solid var(--hell-main);
      margin-bottom: 20px;
      transition: all 0.3s ease;
      box-shadow: 0 0 15px rgba(255, 30, 30, 0.4);
    }
    
    .pixel-img:hover {
      transform: scale(1.05);
      border-color: var(--hell-light);
      box-shadow: 0 0 25px rgba(245, 247, 73, 0.6);
    }
    
    .pixel-btn {
      font-family: 'Press Start 2P', cursive;
      font-size: 0.8rem;
      padding: 10px 20px;
      background-color: var(--hell-main);
      color: white;
      border: 3px solid var(--hell-light);
      text-transform: uppercase;
      transition: all 0.3s ease;
      margin: 5px;
      display: inline-block;
    }
    
    .pixel-btn:hover {
      background-color: var(--hell-accent);
      color: #000;
      transform: translateY(-3px);
      box-shadow: 0 5px 0 var(--hell-light);
    }
    
    .role-card {
      background-color: rgba(138, 3, 3, 0.3);
      border: 3px solid var(--hell-main);
      padding: 15px;
      margin-bottom: 20px;
      transition: all 0.3s ease;
    }
    
    .role-card:hover {
      background-color: rgba(138, 3, 3, 0.5);
      border-color: var(--hell-light);
      transform: translateY(-5px);
    }
    
    .role-name {
      font-family: 'Press Start 2P', cursive;
      font-size: 1rem;
      color: var(--hell-light);
      margin-bottom: 5px;
    }
    
    .pixel-footer {
      background: linear-gradient(to right, var(--hell-main), var(--hell-dark));
      padding: 20px;
      text-align: center;
      font-family: 'Press Start 2P', cursive;
      font-size: 0.7rem;
      border-top: 4px solid var(--hell-accent);
    }
    
    /* Efecto de pixelado */
    .pixel-overlay {
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background-image: 
        linear-gradient(to right, rgba(0,0,0,0.05) 1px, transparent 1px),
        linear-gradient(to bottom, rgba(0,0,0,0.05) 1px, transparent 1px);
      background-size: 4px 4px;
      pointer-events: none;
      z-index: 1000;
    }
    
    @media (max-width: 768px) {
      .game-title {
        font-size: 2rem;
      }
      
      .game-tagline {
        font-size: 0.7rem;
      }
      
      .section-title {
        font-size: 1.2rem;
      }
    }
  </style>
</head>
<body>
  <div class="pixel-overlay"></div>
  
  <div class="pixel-container">
    <!-- Header -->
    <div class="pixel-header">
      <h1 class="game-title">RAILWAY TO HELL</h1>
      <p class="game-tagline">"Death is not the last stop"</p>
    </div>
    
    <!-- Trailer Section -->
    <div class="pixel-section">
      <h2 class="section-title">TRAILER</h2>
      <div class="ratio ratio-16x9">
        <iframe src="https://www.youtube.com/embed/YOUR_VIDEO_ID" allowfullscreen></iframe>
      </div>
      <p class="pixel-text text-center mt-3">"Death is not the last stop"</p>
    </div>
     <!-- DESCRIPTION -->
    <div class="pixel-section">
      <h2 class="section-title">DESCRIPTION</h2>
      <p class="pixel-text"> Railway to Hell is a Metroidvania action and exploration video game set in a hellish version of a railway system. The player controls Nadia, a girl from a gang trapped in this hell, seeking redemption and the chance to escape with her deceased sister. Through intense melee combat, distance combat, progressive skill upgrades and exploration of the train lines, you must challenge the demonic lords who control this hellish system.
</p>
    </div>

    <!-- Narrative Section -->
    <div class="pixel-section">
      <h2 class="section-title">NARRATIVE</h2>
      <p class="pixel-text"> Nadia finds herself in a hellish railway system where demonic lords control different lines. Guided by what she believes are her sister's memories, she must battle through the system, uncovering dark truths about her past and the nature of this underworld.</p>
    </div>
    
    <!-- Game Objective -->
    <div class="pixel-section">
      <h2 class="section-title">GAME OBJECTIVES</h2>
      <ul class="pixel-list">
        <li>Explore interconnected tunnels and stations</li>
        <li>Defeat demonic bosses to gain new abilities</li>
        <li>Solve environmental puzzles</li>
        <li>Uncover the truth about your sister and this hellish world</li>
      </ul>
    </div>
    
    <!-- Art Style -->
    <div class="pixel-section">
      <h2 class="section-title">ART STYLE</h2>
 <div class="row">
        <div class="col-md-6">
          <img src="assets/art-style-1.png" class="img-fluid pixel-img" alt="Estilo de arte pixelado">
        </div>
        <div class="col-md-6">
          <img src="assets/art-style-2.png" class="img-fluid pixel-img" alt="Personajes pixelados">
        </div>
      </div>
    
<p> Our pixel-art aesthetic combines gritty urban decay with supernatural elements, inspired by:</p>
 <ul class="pixel-list">
        <li>Classic Metroidvania games</li>
        <li>Underground graffiti culture</li>
        <li>Gothic and demonic mythology</li>
        <li>Barcelona's unique architecture (like Gaudí's mosaics)</li>
      </ul>
    </div>


    
 <!-- Team -->
<div class="pixel-section">
  <h2 class="section-title">TEAM</h2>
  
  <!-- Primera fila -->
  <div class="row">
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">ANA ALCARAZ</h3>
        <p class="pixel-text">PROGRAMADOR PRINCIPAL</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">JAUME BENEJAM</h3>
        <p class="pixel-text">Diseñador de Juego</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">NOA CABEZA</h3>
        <p class="pixel-text">Directora de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">LAIA CANALETA</h3>
        <p class="pixel-text">Diseñador de Niveles</p>
      </div>
    </div>
  </div>
  
  <!-- Segunda fila -->
  <div class="row">
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">JÚLIA CORNEJO</h3>
        <p class="pixel-text">Directora de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">VÍCTOR GONZÁLEZ</h3>
        <p class="pixel-text">Director de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">ABRIL HERNÁDEZ</h3>
        <p class="pixel-text">Directora de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">HAOSHENG LI</h3>
        <p class="pixel-text">Director de Arte</p>
      </div>
    </div>
  </div>
  
  <!-- Tercera fila -->
  <div class="row">
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">SAÜC PELLEJERO</h3>
        <p class="pixel-text">Director de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">BEL PERANAU</h3>
        <p class="pixel-text">Directora de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">CLAUDIA RUIZ</h3>
        <p class="pixel-text">Directora de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">FELIPE SANCHEZ</h3>
        <p class="pixel-text">Director de Arte</p>
      </div>
    </div>
  </div>
  
  <!-- Cuarta fila -->
  <div class="row">
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">PABLO SANJOSE</h3>
        <p class="pixel-text">Director de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">MARTINA SARRIAS</h3>
        <p class="pixel-text">Directora de Arte</p>
      </div>
    </div>
    <div class="col-md-3">
      <div class="role-card">
        <h3 class="role-name">JAVIER VIDA</h3>
        <p class="pixel-text">Director de Arte</p>
      </div>
    </div>
  </div>
</div>

<!-- CHARACTERS Section -->
<div class="pixel-section">
  <h2 class="section-title">CHARACTERS</h2>

  <h3 class="pixel-text" style="color: var(--hell-accent);">Nadia</h3>
  <p class="pixel-text">
    The main character of the game. A strong, smart and short-tempered woman who has been raised on the streets, part of a gang and a role model to her younger sister, who has recently passed away due to Nadia’s mistakes. Now, dead, she founds herself being unfairly judged by the overlords of hell, who as a punishment won’t let her see her sister, a fatal mistake, because Nadia will travel through all hell for her sister, and she won’t doubt to defeat anyone who stands in her way, be a rat or the lords of hell themselves.
  </p>
  <img src="assets/nadia.png" class="img-fluid pixel-img" alt="Nadia">

  <h3 class="pixel-text" style="color: var(--hell-accent);">Alia</h3>
  <p class="pixel-text">
    Nadia’s little sister. Alia looks up to her older sister, treasuring any activity they get to do together as a memory to remember. She has been lost and trapped in hell since the accident where she lost her life, but with the arrival of her sister, memories which had been taken from her have started to come back, and her stay in hell is not going to be long.
  </p>
  <img src="assets/alia.png" class="img-fluid pixel-img" alt="Alia">

  <h3 class="pixel-text" style="color: var(--hell-accent);">Caronte</h3>
  <p class="pixel-text">
    Caronte serves as the first face Nadia meets in hell, her jailer, keeper of the keys. Caronte is charged with keeping Nadia on a Wagon for all eternity, but no-one would have guessed she was such a menace, at least not poor Caronte.
  </p>
  <img src="assets/caronte.png" class="img-fluid pixel-img" alt="Caronte">

  <h3 class="pixel-text" style="color: var(--hell-accent);">Bosses</h3>

  <h4 class="pixel-text" style="color: var(--hell-accent);">Noma</h4>
  <p class="pixel-text">
    Noma, demon lord representing wrath and leader of the electrical underground line. She carries a copper whip which she uses as a weapon and has control over electricity. As demon of wrath, she doesn’t have much patience and shows an explosive temperament. She will be the first of the overlords to face Nadia on her rescue mission.
  </p>
  <img src="assets/noma.png" class="img-fluid pixel-img" alt="Noma">

  <h4 class="pixel-text" style="color: var(--hell-accent);">Asmodeus</h4>
  <p class="pixel-text">
    Asmodeus, demon lord representing lust and leader of the icy line. He’s a professional clown, proving his abilities in ice skating and juggling. He is fast to devalue the other demon lords, talking at their backs, since he thinks of himself as the only competent one. And, although being the demon of lust, he only finds enjoyment in the beauty of ice.
  </p>
  <img src="assets/asmodeus.png" class="img-fluid pixel-img" alt="Asmodeus">

  <h4 class="pixel-text" style="color: var(--hell-accent);">Belfegor</h4>
  <p class="pixel-text">
    Belfegor, demon lord representing sloth and leader of the poisonous underground line. He is a demon made of smoke and one to truly enjoy the pleasures of privileges from being a demon lord. He will prove to be the embodiment of sloth when Nadia starts killing demons and lords, because he is not going to be showing his face.
  </p>
  <img src="assets/belfegor.png" class="img-fluid pixel-img" alt="Belfegor">

  <h4 class="pixel-text" style="color: var(--hell-accent);">Satanas</h4>
  <p class="pixel-text">
    Satanas is the missing fourth demon lord of hell. The most powerful of the lords and the embodiment of envy. He has a lust for power, and he seeks the fall of the other lords in order to be the one and only lord of hell. And now, with the arrival of Nadia, he may have just found the way to achieve his goal.
  </p>
  <img src="assets/satanas.png" class="img-fluid pixel-img" alt="Satanas">

  <h3 class="pixel-text" style="color: var(--hell-accent);">Enemies</h3>
  <p class="pixel-text">
    Hell’s tunnels are filled with various creatures who, under the orders of the demon lords, are looking to kill Nadia. Rats, bats, spiders and even bombs, all will try to keep Nadia from escaping the grasp of hell.
  </p>
  <img src="assets/enemies.png" class="img-fluid pixel-img" alt="Enemies">
</div>

<!-- ITEMS Section -->
<div class="pixel-section">
  <h2 class="section-title">ITEMS</h2>
  <p class="pixel-text">
    While exploring hell, Nadia will find various items and collectibles that will help her in achieving her goal of escaping hell and rescuing Alia. Some of these items are going to be power-ups that will bestow powerful abilities upon our protagonist, allowing her to jump midair or to dash faster than humanly possible. Others are going to be obtainable only by defeating the demon lords, these being the powerful weapons used by them like the copper whip, able to activate electrical doors, and the ice juggling balls, able to be used as projectiles to defeat enemies from afar. And finally, there are the collectibles, which are shown by Alia’s treasured memories lost throughout hell.
  </p>
  <img src="assets/items.png" class="img-fluid pixel-img" alt="Items">
</div>

<!-- MAPS Section -->
<div class="pixel-section">
  <h2 class="section-title">MAPS</h2>
  <p class="pixel-text">
    Hell has been completely remodeled to look like the most suffer inducing place in the world, the train. And its layout is now that of an underground, each demon lord having control of a line which represents their powers, resulting in the existence of the electrical line and the icy line. Each of them has their own tunnels where enemies roam and stations which hold powerful abilities or the demon lords themselves. And all lines are connected to the Central Station, Satanas’ creation and the only way out of hell.
  </p>
  <img src="assets/maps.png" class="img-fluid pixel-img" alt="Maps">
</div>

<!-- DECISIONS AND ENDINGS Section -->
<div class="pixel-section">
  <h2 class="section-title">DECISIONS AND ENDINGS</h2>
  <p class="pixel-text">
    Due to being a metroidvania game, freedom of exploration is a huge part of the genre. And Railway To Hell isn’t an exception, thanks to the underground layout, the player has complete freedom in how to explore the tunnels and surpass the obstacles that may be on their path, which may affect in how each player experiences the game. Also, there are two different endings depending on the choices made by the player during their playthrough, which will vary depending on if they have collected all of Alia’s memories or not.
  </p>
</div>
<!-- CHARACTERS Section -->
<div class="pixel-section">
  <h2 class="section-title">CHARACTERS</h2>

  <h3 class="pixel-text" style="color: var(--hell-accent);">Nadia</h3>
  <p class="pixel-text">
    The main character of the game. A strong, smart and short-tempered woman who has been raised on the streets, part of a gang and a role model to her younger sister, who has recently passed away due to Nadia’s mistakes. Now, dead, she founds herself being unfairly judged by the overlords of hell, who as a punishment won’t let her see her sister, a fatal mistake, because Nadia will travel through all hell for her sister, and she won’t doubt to defeat anyone who stands in her way, be a rat or the lords of hell themselves.
  </p>
  <img src="assets/nadia.png" class="img-fluid pixel-img" alt="Nadia">

  <h3 class="pixel-text" style="color: var(--hell-accent);">Alia</h3>
  <p class="pixel-text">
    Nadia’s little sister. Alia looks up to her older sister, treasuring any activity they get to do together as a memory to remember. She has been lost and trapped in hell since the accident where she lost her life, but with the arrival of her sister, memories which had been taken from her have started to come back, and her stay in hell is not going to be long.
  </p>
  <img src="assets/alia.png" class="img-fluid pixel-img" alt="Alia">

  <h3 class="pixel-text" style="color: var(--hell-accent);">Caronte</h3>
  <p class="pixel-text">
    Caronte serves as the first face Nadia meets in hell, her jailer, keeper of the keys. Caronte is charged with keeping Nadia on a Wagon for all eternity, but no-one would have guessed she was such a menace, at least not poor Caronte.
  </p>
  <img src="assets/caronte.png" class="img-fluid pixel-img" alt="Caronte">

  <h3 class="pixel-text" style="color: var(--hell-accent);">Bosses</h3>

  <h4 class="pixel-text" style="color: var(--hell-accent);">Noma</h4>
  <p class="pixel-text">
    Noma, demon lord representing wrath and leader of the electrical underground line. She carries a copper whip which she uses as a weapon and has control over electricity. As demon of wrath, she doesn’t have much patience and shows an explosive temperament. She will be the first of the overlords to face Nadia on her rescue mission.
  </p>
  <img src="assets/noma.png" class="img-fluid pixel-img" alt="Noma">

  <h4 class="pixel-text" style="color: var(--hell-accent);">Asmodeus</h4>
  <p class="pixel-text">
    Asmodeus, demon lord representing lust and leader of the icy line. He’s a professional clown, proving his abilities in ice skating and juggling. He is fast to devalue the other demon lords, talking at their backs, since he thinks of himself as the only competent one. And, although being the demon of lust, he only finds enjoyment in the beauty of ice.
  </p>
  <img src="assets/asmodeus.png" class="img-fluid pixel-img" alt="Asmodeus">

  <h4 class="pixel-text" style="color: var(--hell-accent);">Belfegor</h4>
  <p class="pixel-text">
    Belfegor, demon lord representing sloth and leader of the poisonous underground line. He is a demon made of smoke and one to truly enjoy the pleasures of privileges from being a demon lord. He will prove to be the embodiment of sloth when Nadia starts killing demons and lords, because he is not going to be showing his face.
  </p>
  <img src="assets/belfegor.png" class="img-fluid pixel-img" alt="Belfegor">

  <h4 class="pixel-text" style="color: var(--hell-accent);">Satanas</h4>
  <p class="pixel-text">
    Satanas is the missing fourth demon lord of hell. The most powerful of the lords and the embodiment of envy. He has a lust for power, and he seeks the fall of the other lords in order to be the one and only lord of hell. And now, with the arrival of Nadia, he may have just found the way to achieve his goal.
  </p>
  <img src="assets/satanas.png" class="img-fluid pixel-img" alt="Satanas">

  <h3 class="pixel-text" style="color: var(--hell-accent);">Enemies</h3>
  <p class="pixel-text">
    Hell’s tunnels are filled with various creatures who, under the orders of the demon lords, are looking to kill Nadia. Rats, bats, spiders and even bombs, all will try to keep Nadia from escaping the grasp of hell.
  </p>
  <img src="assets/enemies.png" class="img-fluid pixel-img" alt="Enemies">
</div>

<!-- ITEMS Section -->
<div class="pixel-section">
  <h2 class="section-title">ITEMS</h2>
  <p class="pixel-text">
    While exploring hell, Nadia will find various items and collectibles that will help her in achieving her goal of escaping hell and rescuing Alia. Some of these items are going to be power-ups that will bestow powerful abilities upon our protagonist, allowing her to jump midair or to dash faster than humanly possible. Others are going to be obtainable only by defeating the demon lords, these being the powerful weapons used by them like the copper whip, able to activate electrical doors, and the ice juggling balls, able to be used as projectiles to defeat enemies from afar. And finally, there are the collectibles, which are shown by Alia’s treasured memories lost throughout hell.
  </p>
  <img src="assets/items.png" class="img-fluid pixel-img" alt="Items">
</div>

<!-- MAPS Section -->
<div class="pixel-section">
  <h2 class="section-title">MAPS</h2>
  <p class="pixel-text">
    Hell has been completely remodeled to look like the most suffer inducing place in the world, the train. And its layout is now that of an underground, each demon lord having control of a line which represents their powers, resulting in the existence of the electrical line and the icy line. Each of them has their own tunnels where enemies roam and stations which hold powerful abilities or the demon lords themselves. And all lines are connected to the Central Station, Satanas’ creation and the only way out of hell.
  </p>
  <img src="assets/maps.png" class="img-fluid pixel-img" alt="Maps">
</div>

<!-- DECISIONS AND ENDINGS Section -->
<div class="pixel-section">
  <h2 class="section-title">DECISIONS AND ENDINGS</h2>
  <p class="pixel-text">
    Due to being a metroidvania game, freedom of exploration is a huge part of the genre. And Railway To Hell isn’t an exception, thanks to the underground layout, the player has complete freedom in how to explore the tunnels and surpass the obstacles that may be on their path, which may affect in how each player experiences the game. Also, there are two different endings depending on the choices made by the player during their playthrough, which will vary depending on if they have collected all of Alia’s memories or not.
  </p>
</div>

    <!-- Controls -->
    <div class="pixel-section">
      <h2 class="section-title">CONTROLES</h2>
      <div class="row">
        <div class="col-md-6">
          <h3 class="pixel-text" style="color: var(--hell-light);">CONTROLLER</h3>
          <ul class="pixel-list">
            <li><strong>Left Stick move:</strong> left/right</li>
            <li><strong>X/A:</strong> Jump</li>
            <li><strong>R1/RB:</strong> Dash</li>
            <li><strong>Square/X:</strong>Melee Atack</li>
            <li><strong>R2 / RT:</strong> Use whip</li>
            <li><strong>L2 / LT:</strong> Throw Balls</li>
           <li><strong>Circle / B:</strong> Use Key (Doors and Interactions), close dialogues and use checkpoints</li>
           <li><strong>Triangle / Y_</strong> Map</li>
          </ul>
        </div>
        <div class="col-md-6">
          <h3 class="pixel-text" style="color: var(--hell-light);">KEYBOARD</h3>
          <ul class="pixel-list">
            <li><strong>A/D:</strong> left/right</li>
            <li><strong>SPACE:</strong> Jump</li>
            <li><strong>SHIFT:</strong> Dash</li>
            <li><strong>J:</strong> Melee Atack</li>
            <li><strong>K:</strong> Use Whip</li>
            <li><strong>L:</strong> Throw Balls</li>
            <li><strong>E:</strong> Use Key (Doors and Interactions), close dialogues and use checkpoints</li>
            <li><strong>M:</strong> Map</li>
          </ul>
    </div>

    <!-- Gallery -->
    <div class="pixel-section">
      <h2 class="section-title">GALERÍA</h2>
      <div class="row">
        <div class="col-md-4">
          <img src="imagenes/concept.jpg" class="img-fluid pixel-img" alt="Concept2">
        </div>
        <div class="col-md-4">
          <img src="assets/concept2.jpg" class="img-fluid pixel-img" alt="Captura de pantalla 2">
        </div>
        <div class="col-md-4">
          <img src="assets/bossfinal.jpg" class="img-fluid pixel-img" alt="Captura de pantalla 3">
        </div>
      </div>
    </div>

    
    <!-- Links -->
    <div class="pixel-section text-center">
      <h2 class="section-title">Links</h2>
      <a href="#" class="pixel-btn">GITHUB</a>
      <a href="#" class="pixel-btn">KIT DE PRENSA</a>
      <a href="#" class="pixel-btn">BLOG DEL DESARROLLO</a>
    </div>
    
    <!-- Footer -->
    <div class="pixel-footer">
      <p>© 2025 RAILWAY TO HELL - LAST STOP STUDIOS</p>
      <p>"DEATH IS NOT THE LAST STOP"</p>
    </div>
  </div>

  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
