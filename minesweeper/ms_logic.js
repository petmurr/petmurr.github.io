// Logic

export const TILE_STATUSES = { // links to css
    HIDDEN: 'hidden',
    MINE: 'mine',
    NUMBER: 'number',
    MARKED: 'marked'
}


export function createBoard(boardSize, numberOfMines) {
    const board = [] // array of rows
    const minePositions = getMinePositions(boardSize, numberOfMines) //array of xy coords randomly

    for (let x = 0; x < boardSize; x++) {
        const row = [] // array of tiles
        for (let y = 0; y < boardSize; y++) {
            
            const element = document.createElement("div")
            element.dataset.status = TILE_STATUSES.HIDDEN

            const tile = {
                element,
                x,
                y,
                mine: minePositions.some(positionsMatch.bind(null, {x,y})),
                get status() { //get this element's status
                    return this.element.dataset.status
                },
                set status(value) { //set this element's status
                    this.element.dataset.status = value
                },
            } // 'object' that we're going to return

            row.push(tile)
        }
        
        board.push(row)
    }

    return board
}

// flag a tile. If it's hidden and not marked, you can mark it. if it's already marked, unmark it.
export function marktile(tile) {
    if (tile.status !== TILE_STATUSES.HIDDEN && tile.status !== TILE_STATUSES.MARKED) { return }

    if (tile.status == TILE_STATUSES.MARKED) {
        tile.status = TILE_STATUSES.HIDDEN
    } else {
        tile.status = TILE_STATUSES.MARKED
    }

}

export function revealTile(board, tile) {
    if (tile.status !== TILE_STATUSES.HIDDEN) { return }

    if (tile.mine) {
        tile.status = TILE_STATUSES.MINE
        return
    }

    tile.status = TILE_STATUSES.NUMBER
    
    const adjTiles = nearbyTiles(board, tile)
    const mines = adjTiles.filter(t => t.mine)
    
    if (mines.length === 0) { // recursivly call revealTile
        // adjTiles.forEach(revealTile.bind(null, board))

        // go thru adjTiles. If it's not a member of "mines", reveal it.
        adjTiles.forEach(function(element) {
            if (notContainsObject(element, mines)) {
                revealTile(board, element)
            }
        });
    } else {
        tile.element.textContent = mines.length
    }
}

export function checkWin(board) {
    return board.every(row => {
        return row.every(tile => {
            return (tile.status === TILE_STATUSES.NUMBER ||
                (tile.mine && (tile.status === TILE_STATUSES.HIDDEN || tile.status === TILE_STATUSES.MARKED))
            )
        })
    })
       
}

export function checkLose(board) {
    return board.some(row => {
        return row.some(tile => {
            return tile.status === TILE_STATUSES.MINE
        })
    })
}

function getMinePositions(boardSize, numberOfMines) {
    const positions = []

    for (let i=0;i<boardSize;i++) {
        for (let j=0;j<boardSize;j++) {
          const position = {
            x: i,
            y: j
          }
          positions.push(position)
        }
      }
      
      const minestoremove = boardSize**2 - numberOfMines
      
      for (let i=0;i<minestoremove;i++) {
        const remove = randomNumber(positions.length - 1)
        positions.splice(remove, 1)
      }

    return positions
}

function randomNumber(max) {
    return Math.floor(Math.random()*max)
  }

function positionsMatch(a,b) {
    return a.x === b.x && a.y === b.y
}

function nearbyTiles(board, { x, y }) { // x,y are from tile, they are destructured
    const tiles = []

    // for each row(x), go thru columns(y) -1, +1
    for (let i=-1; i<=1; i++) {
        for (let j=-1; j<=1; j++) {

            const tile = board[x+i]?.[y+j] // ?. "if we have something for the thing before, continue"
            if (tile) tiles.push(tile)

        }
    }

    return tiles
}

function notContainsObject(obj, list) {
    var i;
    for (i = 0; i < list.length; i++) {
        if (list[i] === obj) {
            return false;
        }
    }

    return true;
}
